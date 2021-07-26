<?php

/**
 * Created by PhpStorm.
 * User: danrhe
 * Date: 28/02/17
 * Time: 15:06
 */
class Doorkeeper
{

    /**
     * This methods triggest the doorkeeper servlet to create FOXML files for the CMDI and all resources and ingest them.
     *
     * triggering is achieved by executing a curl 'put' response to the service IP
     *
     * @param string $sipId The SIP id
     *
     * @param bool $to Modifier for the ingest procedure. Defines until which action the doorkeeper will run
     *
     * @throws IngestServiceException
     */
    public function triggerServlet($sipId, $to=NULL, $namespace = NULL, $parentFid = NULL) {

        $config = variable_get('flat_deposit_doorkeeper');
        $config_general = variable_get('flat_deposit_general');
        if (is_null($namespace) && is_null($parentFid)) {
        // Use default namespace from config, should not really be happening as there should always be a parent
        	$namespace = $config_general['namespace'];
        }
        elseif (is_null($namespace)) {
        // If no namespace is explicitly passed, use the namespace from the parent object
        	$arr = explode(':', $parentFid, 2);
        	$namespace = $arr[0];
        }

        /*
        if($to){
            $query = '?to=' . $to ;
        } else {
            $query = '';
        }
        */

        $query_params = [];
        $query_params['to'] = $to;
        $query_params['ns'] = $namespace;
        $query = http_build_query($query_params);
        if ($query) {
            $query = '?' . $query;
        }
        else {
            $query = '';
        }

        $ch = curl_init();
        curl_setopt($ch, CURLOPT_URL, $config['url'] . $sipId . $query);
        curl_setopt($ch, CURLOPT_PORT, $config['port']);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
	curl_setopt($ch, CURLOPT_USERPWD, "flat:sword");
        curl_setopt($ch, CURLOPT_PUT, 1);

        $val = curl_exec($ch);

        $httpcode = curl_getinfo($ch, CURLINFO_HTTP_CODE);

        curl_close($ch);

        if ($httpcode != 200 && $httpcode != 202 ) {
            $message = 'Error triggering doorkeeper' . $val;
            throw new IngestServiceException ($message);
        } else {
            return TRUE;
        }
    }


    /**
     * HTTP GET Request for doorkeeper REST api
     *
     * @param string $sipId The SIP id
     *
     * @param bool $code_only if true method returns only HTTP response code
     *
     * @return mixed
     */
    public function doGetRequest($sipId, $code_only=FALSE)
    {

        $config = variable_get('flat_deposit_doorkeeper');

        $ch = curl_init();
        curl_setopt($ch, CURLOPT_URL, $config['url'] . $sipId);
        curl_setopt($ch, CURLOPT_PORT, $config['port']);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
	curl_setopt($ch, CURLOPT_USERPWD, "flat:sword");

        $val = curl_exec($ch);
        $httpcode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
        curl_close($ch);

        $return = $code_only ? $httpcode : $val;
        return $return;
    }


    /**
     * Method requesting every 10 seconds doorkeeper status
     *
     *
     * @param string $sipId the SIP id
     *
     * @param int $maxWait number of seconds the client should wait for doorkeeper to finish request
     *
     * todo check return value of doorkeeper
     */
    public function checkStatus(String $sipId, int $maxWait = 1800){

        #initial check request
        $httpcode = $this->doGetRequest($sipId, TRUE);
        $time = 0;

        // loop and wait until doorkeeper signals end of request
        while ($httpcode != 200 && $time <= $maxWait) {
            sleep(5);
            $time = $time + 5;
            $httpcode = $this->doGetRequest($sipId, TRUE);
        };

        // Check time criterion
        if ($time >= $maxWait){
            $message = "Max time of $maxWait seconds exceeded; Doorkeeper does not finish";
            throw new IngestServiceException ($message);
        }

        // check outcome doorkeeper
        $val = $this->doGetRequest($sipId);
        $xml =simplexml_load_string($val) ;


        if ($xml['status'] != 'succeeded'){
            $message = 'Doorkeeper error' . $val;
            throw new IngestServiceException ($message);
        } else {
            $fid = $xml['fid'];

            return $fid;
        }

    }

}
