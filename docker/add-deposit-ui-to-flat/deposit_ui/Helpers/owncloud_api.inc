<?php
define('OC_ADMIN', variable_get('owncloud_settings',array())['admin_name']);
define('OC_PASS', variable_get('owncloud_settings',array())['admin_pass']);
define('OC_ROOT', variable_get('owncloud_settings',array())['root_dir']);
#define('OC_HOST', variable_get('owncloud_settings',array())['host']);
define('OC_HOST', $_SERVER['HTTP_HOST']);

class OwnCloudConnectionException extends Exception {}

class OwnCloud {

    /* owncloud credentials
     * @var admin:password
     *
    */
    public $admin_name;
    public $admin_pass;
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

    /* Constructor function
    *
    */
    public function __construct($user = NULL) {
        $this->admin_name = OC_ADMIN;
        $this->admin_pass = OC_PASS;
        $this->root_dir = OC_ROOT;

        if (!isset($user)) {
            global $user;
            if ($user->uid == 0) {
                $this->user_name = 'anonymous';
            }
            else {
                $this->user_name = $user->name;
            }
        }

        $this->inUse = variable_get('flat_deposit_modules',array())['owncloud'];

        // if owncloud is used check for existence account; else always true
        $this->noErrors = $this->inUse ? self::owncloud_account_exists() : TRUE;
    }

    public function owncloud_account_exists()
    {
        $oc_url = 'http://' . $this->admin_name . ':' . $this->admin_pass . '@' . OC_HOST . '/owncloud/ocs/v1.php/cloud/users/' . $this->user_name;

        $options = array(
            'max_redirects' => 3,);

        $result = drupal_http_request($oc_url, $options);

        try {
            if (owncloud_check_connection($result)) {
                $xml = new SimpleXMLElement(file_get_contents($oc_url));

                if (owncloud_check_user($xml)){
                    return true;
                } else {
                    throw new OwnCloudConnectionException("Owncloud user account doesn't exist. Login into owncloud account or ask the administrator");
                }
            }else{
                throw new OwnCloudConnectionException('Owncloud server error code (' . $result->code  . ')');}
        }

        catch (OwnCloudConnectionException $e) {
            $this->error_message = $e->getMessage();
            return false;
    }



    }

    public function UpdateUserFiles()
    {
        if ($this->inUse && $this->noErrors) {
            $occ_command = 'php ' . $this->root_dir . '/occ  files:scan --path="/' . $this->user_name . '/files"';
            shell_exec($occ_command);
        }
    }

    /**
     * @param $collection_name  name of the collection which is used to create new directory in owncloud_user directory
     */
    public function CreateCollectionDirectoryAndSync($collection_name){

        if ($this->inUse && $this->noErrors) {
            $full_path = $this->root_dir . 'data/' . $this->user_name . '/files/' . $collection_name;
            if (!file_exists(drupal_realpath($full_path))){
                drupal_mkdir($full_path);
                $this::UpdateUserFiles();
            };

        }
    }

    /**
     * @param $collection_name  name of the collection which is used to create new directory in owncloud_user directory
     */
    public function DeleteCollectionDirectoryAndSync($collection_name){

        if ($this->inUse && $this->noErrors) {
            $full_path = $this->root_dir . 'data/' . $this->user_name . '/files/' . $collection_name;

            if (count(scandir($full_path)) == 2){
                drupal_rmdir($full_path);
                $this::UpdateUserFiles();
            };

        }
    }

    /**
     * Create as symbolic link pointing from the project folder (which is located in the drupal user data directory)
     * to the collection folder in owncloud
     *
     * @param $user_data_dir drupal path to the users data directory (e.g. public://users/nameUser)
     * @param $collection name of the collection the bundle belongs to
     * @param $bundle name of the upload bundle
     */
    public function CreateProjectSymlinkAndSync($user_data_dir, $collection, $bundle){

        if ($this->inUse && $this->noErrors) {
            $link_name = $this->root_dir . 'data/' . $this->user_name . "/files/$collection/$bundle";
            $target = drupal_realpath("$user_data_dir/$collection/$bundle/data");
            symlink ($target, $link_name);
            $this::UpdateUserFiles();
        }
    }

    public function DeleteProjectSymlinkAndSync($collection, $bundle){

        if ($this->inUse && $this->noErrors) {
            $link_name = $this->root_dir . 'data/' . $this->user_name . "/files/$collection/$bundle";

            if(is_link($link_name)) {
                drupal_unlink($link_name);
            } else {
                drupal_set_message(t("$link_name exists but not symbolic link\n", array()), 'error');
            }
            $this::UpdateUserFiles();
        }
    }

    public function DeleteBrokenLinks(){

        if ($this->inUse && $this->noErrors) {
            $folder_name = $this->root_dir . 'data/' . $this->user_name . "/files/";
            $command = "find -L $folder_name -type l -delete";
            exec($command);
            $this::UpdateUserFiles();
        }
    }

}


function owncloud_check_connection($result) {
    if ($result->code == "200") {
        return true;
    } else {
        return false;
    }
}

function owncloud_check_user($xml)
{
    if ($xml->data->enabled) {
        return true;
    } else {
        return false;
    }
}



