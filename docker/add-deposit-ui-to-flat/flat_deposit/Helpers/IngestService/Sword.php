<?php


class Sword
{

    /**
     * @param String $pathToSip directory containing the directory with the original files and a zipped SIP
     * @param String $zipName name of the SIP
     * @param String $SipId
     * @return bool
     * @throws IngestServiceException
     */
    function postSip($pathToSip, $zipName, $sipId)
    {
        $cwd = getcwd();

        chdir($pathToSip);

        $data = file_get_contents($zipName);
        $md5 = md5_file($zipName);

        $bagId = $sipId . '_sword';

        $config = variable_get('flat_deposit_sword');
        $ch = curl_init();

        $url = $config['url'].'collection';
        #$url = 'http://localhost/flat/easy-deposit/collection';

        $port = $config['port'];
        #$port = '80';

        curl_setopt($ch, CURLOPT_URL, $url);
        curl_setopt($ch, CURLOPT_PORT, $config['port']);
        curl_setopt($ch, CURLOPT_USERPWD, sprintf("%s:%s", $config['user'], $config['password']));
        curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
        curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, false);

        curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
        curl_setopt($ch, CURLOPT_BINARYTRANSFER, TRUE);
        curl_setopt($ch, CURLOPT_POST, 1);
        curl_setopt($ch, CURLOPT_POSTFIELDS, $data);

        curl_setopt($ch, CURLOPT_HEADER, TRUE);
        curl_setopt($ch, CURLOPT_HTTPHEADER, array(
            'Content-Type: application/zip',
            'Content-Disposition: attachment; filename='.$zipName,
            'Content-MD5: '.$md5,
            'Packaging: http://purl.org/net/sword/package/BagIt',
            'Slug: '.$sipId,
            'In-Progress: false'));

        $content = curl_exec($ch);
        $httpcode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
        curl_close($ch);

        chdir($cwd);

        if ($httpcode != 200 && $httpcode != 202 && $httpcode != 201) {
            $message = sprintf("SWORD Server error (HTTP error code (%d) ;\n", $httpcode) . $content;
            throw new IngestServiceException($message);
        } else {
            return TRUE;
        }
    }


    /**
     * HTTP GET Request for SWORD REST api
     *
     * @param String $bagId the bag ID
     *
     * @param bool $code_only if true method returns only HTTP response code
     * @return mixed
     */
    function getRequest($bagId, $code_only = FALSE)
    {
        $config = variable_get('flat_deposit_sword');
        $ch = curl_init();

        curl_setopt($ch, CURLOPT_URL, $config['url'] . 'statement/' . $bagId);
        curl_setopt($ch, CURLOPT_PORT, $config['port']);
        curl_setopt($ch, CURLOPT_USERPWD, sprintf("%s:%s", $config['user'], $config['password']));
        curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
        curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, false);

        curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);

        $val = curl_exec($ch);
        $httpcode = curl_getinfo($ch, CURLINFO_HTTP_CODE);

        curl_close($ch);

        return $code_only ? $httpcode : $val;
    }


    /**
     * Create Bag at correct bag location
     *
     *
     * @throws IngestServiceException
     */

    function checkStatus($bagId)
    {

        #initial check request
        $val = $this->getRequest($bagId);
        $xml =simplexml_load_string($val) ;
        $status = (string)$xml->category['term'];

        // loop and wait until SWORD signals end of request
        while ($status == 'FINALIZING') {
            sleep(2);
            $val = $this->getRequest($bagId);
            $xml =simplexml_load_string($val) ;
            $status = (string)$xml->category['term'];
        };

        // check outcome SWORD
        if ($status != 'SUBMITTED') {
            $message = "Error creating bag;\n" .  $val;
            throw new IngestServiceException($message);
        } else {
            return TRUE;
        }
    }

    /**
     * Wait until Doorkeeper is done and Sword status is REJECTED
     *
     * Rollback should not start before this, as the Doorkeeper rollback may still be ongoing
     *
     */
    function swordRejected($sipId)
    {
        $bagId = $sipId . '_sword';

        $time = 0;

        #initial check request
        $val = $this->getRequest($bagId);
        $xml = simplexml_load_string($val);
        $status = (string)$xml->category['term'];

        // loop and wait until SWORD signals end of request
        while ($status == 'FINALIZING') {
            sleep(2);
            $time = $time + 2;
            $val = $this->getRequest($bagId);
            $xml = simplexml_load_string($val);
            $status = (string)$xml->category['term'];
            if ($time >= 600) {
                return false;
            }
        };

        // check outcome SWORD
        if ($status = 'REJECTED') {
            return true;
        }
    }
}
