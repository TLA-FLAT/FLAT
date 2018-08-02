<?php

/**
 * Created by PhpStorm.
 * User: danrhe
 * Date: 17/08/2017
 * Time: 10:04
 */
class AclHandlerNc
{

    private $header =[];
    private $info = [];


    /**
     * AclHandler constructor.
     *
     * @param $fName string containing full path to file or folder
     *
     * @return AclHandler
     */
    public function __construct($fName)
    {

        // read out acl info
        $acl_string = shell_exec("getfacl \"$fName\"");

        if (empty($acl_string)){
            return null;

        }


        // transpose string output into array (eliminate empty entries)
        $acl_array = array_filter(explode("\n", $acl_string));

        $this->_setHeader($acl_array);

        // transpose acl info into associative array
        foreach ($acl_array as $item){

            $str_array = explode(':',$item);
            $class = $this::getEntryType($str_array);
            $this::_setAccess($class, $str_array);

        }



        return $this;
    }


    private function _setHeader(&$acl_array){
        $this->header ['name'] = str_replace('# file: ', '', $acl_array[0]);
        $this->header ['owner'] = str_replace('# owner: ', '', $acl_array[1]);
        $this->header ['group'] = str_replace('# group: ', '', $acl_array[2]);

        unset($acl_array[0]);
        unset($acl_array[1]);
        unset($acl_array[2]);

	    foreach ($acl_array as $key => $string) {
 		    if (strpos($string, '# flags: ') === 0) {
    			$this->header ['flags'] = str_replace('# flags: ', '', $acl_array[$key]);
			unset($acl_array[$key]);
  		    }
	    }
    }

    public function getInfo(){

        return $this->info;

    }

    public function getHeader(){

        return $this->header;

    }


    /**
     * Reads acl info to determine entry type
     *
     * @param $permission_entry array array-tified row of acl info of the form type:{(optional)name}:permissions (e.g. user::rwx)
     *
     * @return string ACL entry type
     */
    private function getEntryType($permission_entry){


        switch ($permission_entry[0]){

            case 'user':

                if (strlen($permission_entry[1]) == 0){

                    $entry_type = 'owner';

                } else{

                    $entry_type = 'named user';

                };
                break;

            case 'group':

                if(strlen($permission_entry[1]) == 0){

                    $entry_type = 'owning group';
                } else{

                    $entry_type = 'named group';

                };
                break;


            default:

                $entry_type = $permission_entry[0];

        }

        return $entry_type;

    }

    /**
     * Checks ACL info in order to put ids with read, write and/or executable permissions into an associative array.
     * Default acl entries are ignored.
     *
     * @param $class string ACL entry type
     *
     * @param $permissions array ACL info entry
     *
     */
    private function _setAccess($class, $permissions){
        $read = strpos($permissions[2],'r') !== FALSE;
        $write = strpos($permissions[2],'w') ? TRUE : FALSE;
        $execute = strpos($permissions[2],'x') ? TRUE : FALSE;

        switch ($class){
            case 'owner':
            case 'named user':

                if ($class== 'owner'){
                    $id = $this->getHeader()['owner'];

                } else {

                    $id = $permissions[1];
                }

                break;

            case 'owning group':
            case 'named group':

                if ($class== 'owning group'){
                    $id = $this->getHeader()['group'];

                } else {

                    $id = $permissions[1];
                }

                break;

            case 'other':
	/*
            case 'mask':

                $id = NULL;
                break;
	*/
            default:
                $read = FALSE;
                $write = FALSE;
                $execute = FALSE;

        }


        if ($read === TRUE) $this->_addToInfo('read', $class, $id);

        if ($write === TRUE) $this->_addToInfo('write', $class, $id);

        if ($execute === TRUE) $this->_addToInfo('execute', $class, $id);

    }

    /**
     * Adds an entry to info attribute in a structured way
     *
     * @param $permission_type string read write or executable
     *
     * @param $entry_type string user group other or mask
     *
     * @param $id string ID to which the permission belongs
     *
     */
    private function _addToInfo($permission_type, $entry_type, $id=NULL ){
        if (isset($id)) {
            $this->info[$permission_type][$entry_type][] = $id;

        } else {
            $this->info[$permission_type][$entry_type] = TRUE;

        }
    }


    public function checkWriteAccess($id){

        // Check if id is owner
        if (is_numeric(array_search($id, $this->info['write']['owner']))){
            return 'ID is owner of file or directory';
        }

        // Check if id is in list of named users
	    if (isset($this->info['write']['named user'] )) {
        	if (is_numeric(array_search($id, $this->info['write']['named user']))){
            		return 'ID is named user of file or directory';
        	}
	    }

        $groups = shell_exec("id -Gn $id");

        if (empty($groups)){
            return false;
        }

        $groups_array = explode(' ', trim($groups));
        // Check if owning group has write permissions and is in list of groups with the specified ID
        if (isset($this->info['write']['owning group']) AND in_array((string)$this->info['write']['owning group'][0], $groups_array)){
            return 'ID is member of owning group of file or directory';
        }

        // Check for each named group if is in list of groups with the specified ID
        if (isset($this->info['write']['named group'] )) {
            foreach ($this->info['write']['named group'] as $group_entry) {

                if (is_numeric(array_search($group_entry, $groups_array))) {
                    return 'ID is member of a named group that has writing permissions for file or directory';
                }
            }
        }
        return false;

    }

public function checkReadAccess($id){

        // Check if id is owner
        if (is_numeric(array_search($id, $this->info['read']['owner']))){
            return 'ID is owner of file or directory';
        }

        // Check if id is in list of named users
	if (isset($this->info['read']['named user'] )) {
        	if (is_numeric(array_search($id, $this->info['read']['named user']))){
            		return 'ID is named user of file or directory';
        	}
	}

        $groups = shell_exec("id -Gn $id");
        if (empty($groups)){
            return false;
        }
        $groups_array = explode(' ', trim($groups));
        // Check if owning group has write permissions and is in list of groups with the specified ID
        if (isset($this->info['read']['owning group'][0]) AND in_array((string)$this->info['read']['owning group'][0], $groups_array)){
            return 'ID is member of owning group of file or directory';
        }

        // Check for each named group if is in list of groups with the specified ID
        if (isset($this->info['read']['named group'] )) {
            foreach ($this->info['read']['named group'] as $group_entry) {

                if (is_numeric(array_search($group_entry, $groups_array))) {
                    return 'ID is member of a named group that has read permissions for file or directory';
                }
            }
        }
        return false;

    }
}