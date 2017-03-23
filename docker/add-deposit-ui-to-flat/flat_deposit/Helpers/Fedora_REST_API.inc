<?php

/*
* Fedora REST API object
*/


/**
 * Class FedoraRESTAPI
 */
class FedoraRESTAPI{

    private $config = array();

    /**
     * FedoraRESTAPI constructor.
     * @param array $config host_ip, port and credentials of fedora server
     */
    public function __construct($config)
    {
        $this->config = $config;
        $this->base_url = $this->config['scheme'] . '://' . $this->config['host_ip'] . '/' . $this->config['context'] . "/objects/";
        $this->status = False;
    }


    private function setDefaultOptions()
    {
        $ch = curl_init();
        curl_setopt_array($ch, array(
            CURLOPT_PORT => $this->config['port'],
            CURLOPT_USERPWD => $this->config['user'] . ':' . $this->config['password'],
            CURLOPT_RETURNTRANSFER => 1,
            CURLOPT_CONNECTTIMEOUT => 5,
            CURLOPT_TIMEOUT => 5,
            CURLOPT_SSL_VERIFYPEER => false, #-k
            CURLOPT_SSL_VERIFYHOST => false)); #-k
        return $ch;
    }


    public function getObjectData($pid){

        $ch = $this->setDefaultOptions();
        curl_setopt($ch, CURLOPT_URL, sprintf("%s?%s", $this->base_url . $pid, 'format=xml'));

        $result = $this->doRequest($ch);
        if ($result) return json_decode(json_encode((array)simplexml_load_string($result)),TRUE);
    }


    public function modifyObject($pid, $fields){
        $ch = $this->setDefaultOptions();
        curl_setopt($ch, CURLOPT_URL, sprintf("%s?%s", $this->base_url . $pid , http_build_query($fields)));
        curl_setopt($ch, CURLOPT_PUT, 1);
        return $this->doRequest($ch);

    }

    public function deleteObject($pid){
        $ch = $this->setDefaultOptions();
        curl_setopt($ch, CURLOPT_URL, $this->base_url . $pid);
        curl_setopt($ch, CURLOPT_CUSTOMREQUEST, "DELETE");
        return $this->doRequest($ch);
    }

    private function doRequest($ch){
        $result = curl_exec($ch);
        $httpcode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
        curl_close($ch);
        return ($httpcode >= 200 && $httpcode < 300) ? $result : false;
    }


}

