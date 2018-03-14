<?php
class DOMValidator
{
    /**
     * @var string
     */
    protected $feedSchema ;
    /**
     * @var int
     */
    public $feedErrors = 0;

    /**
     * Formatted libxml Error details
     *
     * @var array
     */
    public $errorDetails;

    /**
     * Validation Class constructor Instantiating DOMDocument
     *
     * @param \DOMDocument $handler [description]
     */
    public function __construct()
    {
        $this->handler = new \DOMDocument('1.0', 'utf-8');
    }


    private function setFeedSchema($feedSchema = null){

        if ($feedSchema){
            $this->feedSchema = __DIR__  .  '/FormTemplates/' . $feedSchema;
        }
    }

    /**
     * @param \libXMLError object $error
     *
     * @return string
     */
    private function libxmlDisplayError($error)
    {
        $errorString = "Error $error->code in $error->file (Line:{$error->line}):";
        $errorString .= trim($error->message);
        return $errorString;
    }

    /**
     * @return array
     */
    private function libxmlDisplayErrors()
    {
        $errors = libxml_get_errors();
        $result    = [];
        foreach ($errors as $error) {
            $result[] = $this->libxmlDisplayError($error);
        }
        libxml_clear_errors();
        return $result;
    }

    /**
     * Validate Incoming Feeds against Listing Schema
     *
     * @param resource $feeds
     *
     * @return bool
     *
     * @throws \Exception
     */
    public function validateFeeds($feeds, $feedSchema = null)
    {
        if (!class_exists('DOMDocument')) {
            throw new \DOMException("'DOMDocument' class not found!");
        }

        $this->setFeedSchema($feedSchema);

        if (!file_exists($this->feedSchema)) {
            throw new \Exception('Schema is Missing, Please add schema to feedSchema property');
        }

        libxml_use_internal_errors(true);

        if (!($fp = fopen($feeds, "r"))) {
            throw new \Exception('Could not open cmdi file');
        }

        $contents = fread($fp, filesize($feeds));
        fclose($fp);

        $this->handler->loadXML($contents, LIBXML_NOBLANKS);
        if (!$this->handler->schemaValidate($this->feedSchema)) {
            throw new Exception('My Error');
                #var_dump($this->libxmlDisplayErrors()));
        }
    }

    /**
     * Display Error if Resource is not validated
     *
     * @return array
     */
    public function displayErrors()
    {
        return $this->errorDetails;
    }
}

