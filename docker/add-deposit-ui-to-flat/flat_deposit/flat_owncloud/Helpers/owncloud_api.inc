<?php

class OwnCloudConnectionException extends Exception {}

class OwnCloud
{

    /* owncloud credentials
     * @var admin:password
     *
    */

    public $root_dir;

    /* Should owncloud be used or not
    *
    */
    public $inUse;

    /* Bool indicating that owncloud didn't produce errors
    *
    */
    public $noErrors;

    /* String containing a possible error message to be displayed in drupal
     *
     */
    public $error_message;

    /* user name
    *
    */
    public $user_name;

    /* Variables related to mounting drupal data directory to owncloud
    *
    */
    public $mounted;
    public $verified;
    public $archive_mount;


    /* Constructor function
    *
    */
    public function __construct($user = NULL)
    {

        $config = variable_get('flat_owncloud',array());

        $this->root_dir = $config['root_dir'];

        $this->mounted = FALSE;
        $this->verified = FALSE;
        $this->archive_mount = [];

        if (!isset($user)) {
            global $user;
            if ($user->uid == 0) {
                $this->user_name = 'anonymous';
            } else {
                $this->user_name = $user->name;
            }
        }

        $this->inUse = variable_get('flat_owncloud', array())['activated'];

        // if owncloud is used check for existence account; else always true
        $this->noErrors = $this->inUse ? self::owncloud_account_exists() : TRUE;
    }

    public function call_user_provision_api($method, $code_only = FALSE)
    {
        switch ($method) {
            case 'get': {

                $config = variable_get('flat_owncloud',array());
                $oc_url = 'http://' . $config['host'] . '/owncloud/ocs/v1.php/cloud/users/' . $this->user_name;
                $ch = curl_init();

                curl_setopt($ch, CURLOPT_URL, $oc_url); //
                curl_setopt($ch, CURLOPT_USERPWD, sprintf("%s:%s", $config['admin_name'], $config['admin_pass']));
                curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE); // -i

                $content = curl_exec($ch);
                $httpcode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
                curl_close($ch);
                break;
            }
        }
        return $code_only ? $httpcode : $content;

    }

    /**
     * Checks for existence of owncloud user account. In case of connection errors an exception is thrown. Otherwise
     *
     * @return bool
     */
    public function owncloud_account_exists()
    {
        try {
            $code = $this->call_user_provision_api('get', TRUE);
            if ($code != 200) {
                throw new OwnCloudConnectionException('Error calling owncloud user provision api ( HTTP code ' . $code . ')');
            }

            $content = $this->call_user_provision_api('get');
            $xml = simplexml_load_string($content);
            if ($xml->data->enabled->__toString()) {
                return true;
            } else {
                throw new OwnCloudConnectionException("Owncloud user account doesn't exist. Login into owncloud account or ask the administrator");
            }
        } catch
        (OwnCloudConnectionException $e) {
            $this->error_message = $e->getMessage();
            return false;
        }
    }

    public function GetOwncloudMounts(){

        $config = variable_get('flat_owncloud',array());

        exec($config['root_dir'] . "/occ files_external:export", $res, $code);

        if ($code){

            throw new OwnCloudConnectionException("Could not get owncloud mounts. Please ask administrator to check owncloud settings");

        }

        return $res;
    }

    /**
     * Compares the directory naming of owncloud mounts (as specified in $mount->configuration->dir) with naming of user drupal data directory
     * @return array of all drupal user directory mounts
     */
    public function ListAllDrupalMounts($test=FALSE)
    {
        // grap all user mount configurations
        $res = $this->GetOwncloudMounts();
        $mounts = json_decode(implode($res));

        $drupal_mounts = [];


        // convert path strings of drupal data directory and each owncloud mount into arrays and count overlap between both
        $d_path = explode("/",drupal_realpath('private://flat_deposit'));

        foreach ($mounts as $mount) {
            $m_path = explode('/',$mount->configuration->datadir); # mounted path is always one field longer (i.e. user acronym) than general drupal data path
            if (count(array_intersect($d_path, $m_path)) == (count($m_path) - 1)){
                $drupal_mounts [] = $mount;
            }
        }

        if ($test){
            drupal_set_message(t('Owncloud listing drupal mounts test passed. ' . (string)count($mounts) . ' mounts found with ' . (string)count($drupal_mounts) . ' drupal mounts'));
        }

        return $drupal_mounts;
    }

    public function MountDataFolder($test=FALSE)
    {
        global $user;
        $config = variable_get('flat_owncloud',array());

        if ($test){

            $datadir = $config['temp_dir'];
            $mpoint = "/MPI_Archive_TEST";
            $message = 'Trying to create owncloud test mount';

        } else {

            global $user;
            $datadir =drupal_realpath('private://flat_deposit/data/'. $user->name);
            $mpoint = "/MPI_Archive_" . $user->name;
            $message = 'Trying to create owncloud user mount';
        }

        $fname = $config['temp_dir'] . "/owncloud_mount_" . $user->name . ".json";

        $mount_config = array(
            array(
                "mount_point" => $mpoint,
                "storage" => "\\OC\\Files\\Storage\\Local",
                "authentication_type" => "null::null",
                "configuration" => array(
                    "datadir" => $datadir),
                "options" => array(
                    "enable_sharing" => false,
                    "encoding_compatibility" => false,
                    "encrypt" => true,
                    "filesystem_check_changes" => 1,
                    "previews" => true),
                "applicable_users" => array(
                    $user->name),
                "applicable_groups" => []
            ));

        file_put_contents($fname, json_encode($mount_config));
        $cmd = $config['root_dir'] . "/occ files_external:import $fname";
        exec($cmd, $res, $code);



#        drupal_set_message(t($message));


        return $code;

    }

    /**
     * Checks if owncloud mounts contain the user drupal data directory.
     * @return bool
     */
    public function GetUserMount($test=FALSE)
    {
        $config = variable_get('flat_owncloud',array());

        if ($test){
            $datadir = $config['temp_dir'];
        } else {
            global $user;
            $datadir =drupal_realpath('private://flat_deposit/data/'. $user->name);
        }

        $res = $this->GetOwncloudMounts();
        $mounts = json_decode(implode($res));

        foreach ($mounts as $mount) {

            if ($mount->configuration->datadir == $datadir) {
                $this->mounted = TRUE;
                $this->archive_mount = $mount;
            }
        }
        return $this->mounted;
    }

    /**
     * Unmounts the owncloud mount for a specified mount id. Mount id can be given manually or assigned during GetUserMount
     * @param null $mount_id
     * @return bool
     */
    public function UnmountUserMount($mount_id = NULL, $test=FALSE)
    {

        $mount_id = $mount_id ? $mount_id : $this->archive_mount->mount_id;

        $config = variable_get('flat_owncloud',array());

        exec($config['root_dir'] . "/occ files_external:delete -y " . $mount_id, $res, $code);

        if ($test){
            drupal_set_message(t('Owncloud test mount removed'));
        }

        return $code ? false : TRUE;
    }





    public function VerifyMount()
    {
        $config = variable_get('flat_owncloud',array());

        exec($config['root_dir'] . "/occ files_external:verify " . $this->archive_mount->mount_id . ' --output=json', $res, $code);

        if (!$code){

            $formatted = json_decode($res);

            if ($formatted['status'] == 'ok') {

                $this->verified = TRUE;

            }
        }


        return $this->verified;

    }




    public function WarnUserMount()
    {

        if ($this->inUse && $this->noErrors) {

            $user_data = user_load($GLOBALS['user']->uid);
            $activated = isset($user_data->mount_owncloud['und'][0]['value']) ? $user_data->mount_owncloud['und'][0]['value'] : '0';

            $link = array(
                'theme' => 'link',
                'text' => 'user account',
                'path' => 'user/' . $GLOBALS['user']->uid . '/edit',
                'options' => array(
                    'attributes' => array(),
                    //REQUIRED:
                    'html' => FALSE,
                ),
            );

            if (is_string($activated) AND $activated == '0') {
                drupal_set_message(t("Owncloud integration has not been activated yet. You can do this in your " . theme_link($link)  . " settings", array()), 'warning');
            }
        }

    }

    public function UpdateUserFiles()
    {
        if ($this->inUse && $this->noErrors) {
            $occ_command = 'php ' . $this->root_dir . '/occ  files:scan --path="/' . $this->user_name . '/files"';
            exec($occ_command);
        }
    }

    /**
     * @param $collection_name  name of the collection which is used to create new directory in owncloud_user directory
     */
    public function CreateCollectionDirectoryAndSync($collection_name)
    {

        if ($this->inUse && $this->noErrors) {
            $full_path = $this->root_dir . 'data/' . $this->user_name . '/files/' . $collection_name;
            if (!file_exists(drupal_realpath($full_path))) {
                drupal_mkdir($full_path);
                $this::UpdateUserFiles();
            };

        }
    }

    /**
     * @param $collection_name  name of the collection which is used to create new directory in owncloud_user directory
     */
    public function DeleteCollectionDirectoryAndSync($collection_name)
    {

        if ($this->inUse && $this->noErrors) {
            $full_path = $this->root_dir . 'data/' . $this->user_name . '/files/' . $collection_name;

            if (count(scandir($full_path)) == 2) {
                drupal_rmdir($full_path);
                $this::UpdateUserFiles();
            };

        }
    }

    /**
     * Create as symbolic link pointing from the project folder (which is located in the drupal user data directory)
     * to the collection folder in owncloud
     *
     * @param $user_data_dir drupal path to the users data directory (e.g. private://flat_deposit/nameUser)
     * @param $collection name of the collection the bundle belongs to
     * @param $bundle name of the upload bundle
     */
    public function CreateProjectSymlinkAndSync($user_data_dir, $collection, $bundle)
    {

        if ($this->inUse && $this->noErrors) {
            $link_name = $this->root_dir . 'data/' . $this->user_name . "/files/$collection/$bundle";
            $target = drupal_realpath("$user_data_dir/$collection/$bundle/data");
            symlink($target, $link_name);
            $this::UpdateUserFiles();
        }
    }

    public function DeleteProjectSymlinkAndSync($collection, $bundle)
    {

        if ($this->inUse && $this->noErrors) {
            $link_name = $this->root_dir . 'data/' . $this->user_name . "/files/$collection/$bundle";

            if (is_link($link_name)) {
                drupal_unlink($link_name);
            } else {
                drupal_set_message(t("$link_name exists but not symbolic link\n", array()), 'error');
            }
            $this::UpdateUserFiles();
        }
    }

    public function DeleteBrokenLinks()
    {

        if ($this->inUse && $this->noErrors) {
            $folder_name = $this->root_dir . 'data/' . $this->user_name . "/files/";
            $command = "find -L $folder_name -type l -delete";
            exec($command);
            $this::UpdateUserFiles();
        }
    }




}
