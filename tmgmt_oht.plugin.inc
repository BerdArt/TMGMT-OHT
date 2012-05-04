<?php

/**
 * @file
 * Provides OneHourTranslation translation plugin controller.
 */

/**
 * OHT translation plugin controller.
 */
class TMGMTOhtPluginController extends TMGMTDefaultTranslatorPluginController {

  /**
   * Translation service URL.
   *
   * @var string
   */
  protected $apiURL = 'https://api.onehourtranslation.com/';
  protected $sandboxURL = 'https://sandbox.onehourtranslation.com/';
  protected $supportedLanguages = array(
    'sq' => 'sq-al',
    'ar' => 'ar-sa',
    'bn' => 'bn-bd',
    'bs' => 'bs-ba',
    'bg' => 'bg-bg',
    'ca' => 'ca-es',
//    '' => 'zh-cn-yue',
    'zn-hans' => 'zh-cn-cmn-s',
    'zn-hant' => 'zh-cn-cmn-t',
    'hr' => 'hr-hr',
    'cs' => 'cs-cz',
    'da' => 'da-dk',
    'fa' => 'fa-af',
    'nl' => 'nl-nl',
    'en-gb' => 'en-uk',
    'en' => 'en-us',
    'et' => 'et-ee',
//    '' => 'fa-ir',
    'fi' => 'fi-fi',
    'fr' => 'fr-fr',
//    '' => 'fr-ca',
    'ka' => 'ka-ge',
    'de' => 'de-de',
    'el' => 'el-gr',
    'he' => 'he-il',
    'hi' => 'hi-in',
    'hu' => 'hu-hu',
    'id' => 'id-id',
    'it' => 'it-it',
    'jp' => 'jp-jp',
    'kk' => 'kk-kz',
//    '' => 'km-kh',
    'ko' => 'ko-kp',
    'lo' => 'lo-la',
    'lv' => 'lv-lv',
    'lt' => 'lt-lt',
    'mk' => 'mk-mk',
    'ms' => 'ms-my',
    'nb' => 'no-no',
    'ps' => 'ps',
    'pl' => 'pl-pl',
    'pt-br' => 'pt-br',
    'pt-pt' => 'pt-pt',
    'ro' => 'ro-ro',
    'ru' => 'ru-ru',
    'sr' => 'sr-rs',
    'sk' => 'sk-sk',
    'sl' => 'sl-si',
    'es' => 'es-es',
//    '' => 'es-ar',
    'sv' => 'sv-se',
    'tl' => 'tl-ph',
    'ta' => 'ta-in',
    'th' => 'th-th',
    'tr' => 'tr-tr',
    'uk' => 'uk-ua',
    'ur' => 'ur',
    'uz' => 'uz-uz',
    'vi' => 'vi-vn'
  );

  /**
   * Implements TMGMTTranslatorPluginControllerInterface::isAvailable().
   */
  public function isAvailable(TMGMTTranslator $translator) {
    if ($translator->getSetting('api_account_id') && $translator->getSetting('api_secret_key')) {
      return TRUE;
    }
    return FALSE;
  }

  /**
   * Implements TMGMTTranslatorPluginControllerInterface::canTranslation().
   *
   * We will check are source and target language supported.
   */
  public function canTranslate(TMGMTTranslator $translator, TMGMTJob $job) {
    // if we use sandbox we can translate anyway
    if($translator->getSetting('use_sandbox')) {
      return TRUE;
    }
    // if Translator is configured - check if the source and target language are supported.
    if ($this->isAvailable($translator)) {
      $targets = $this->getSupportedLanguages();
      return isset($targets[$job->source_language]) && isset($targets[$job->target_language]);
    }
    return FALSE;
  }

  /**
   * Implements TMGMTTranslatorPluginControllerInterface::requestTranslation().
   *
   * Here we will acutally query source and get translations.
   */
  public function requestTranslation(TMGMTJob $job) {
    // Pull the source data array through the job and flatten it.
    $data = $this->prepareDataForSending($job);
    $translator = $job->getTranslator();
    $languages = $this->getSupportedLanguages();

    // request translation from OHT service
    $response = $this->doRequest($job->getTranslator(), 'api/1/project/new', array(
      'source' => $translator->getSetting('use_sandbox') ? 'en-us' : $languages[$job->source_language],
      'target' => $translator->getSetting('use_sandbox') ? 'fr-fr' : $languages[$job->target_language],
      'source_text' => $data,
      'callback_url' => url('tmgmt_oht_callback', array('absolute' => TRUE)),
      'custom0' => $job->tjid,
    ), 'POST');

    // handling reponse
    $response_data = json_decode($response->data);

    if ($response->code == '200' && empty($response_data->status_code)) {
      $job->submitted('Job has been successfully submitted for translation. Project ID is: <pre>@project_id</pre>',
        array('@project_id' => $response_data->project_id));
      $job->addMessage('@credits credits reduced from your account.', array('@credits' => $response_data->credits));
      $job->addMessage('Response data: <br /><pre>@response</pre>', array('@response' => print_r($response_data, 1)));
      $job->reference = $response_data->project_id;
    }
    else {
      $job->rejected('Could not send Job to OHT. Message error: @error',
        array('@error' => $response_data->status_msg), 'error');
    }
  }

  /**
   * Receives and stores a translation returned by OHT.
   */
  public function receiveTranslation(TMGMTJob $job, $data) {
    $data = $this->parseTranslationData($data);
    $job->finished('The translation has been received.');
    $job->addTranslatedData($data);
  }

  /**
   * Returns list of languages supported by OHT service
   * @param TMGMTTranslator $translator
   * @param $source_language
   * @return array
   */
  public function getSupportedTargetLanguages(TMGMTTranslator $translator, $source_language) {
    return drupal_map_assoc(array_keys($this->supportedLanguages));
  }

  /**
   * Returns array of supported languages by OHT service
   * @return array
   * Returns array of the supported languages.
   * The key is drupal-style language code, the value - OHT-style language code.
   */
  protected function getSupportedLanguages() {
    return $this->supportedLanguages;
  }

  /**
   * Execute a request against the OHT API.
   *
   * @param TMGMTTranslator $translator
   *   The translator entity to get the settings from.
   * @param $path
   *   The path that should be appended to the base uri, e.g. Translate or
   *   GetLanguagesForTranslate.
   * @param $query
   *   (Optional) Array of GET query arguments.
   * @param $options
   *   (Optional) Array of additional options passed to drupal_http_request().
   *
   * @return
   *   The response object returned by drupal_http_request().
   */
  protected function doRequest(TMGMTTranslator $translator, $path, array $query = array(), $method = 'GET') {
    $options = array();

    if ($custom_url = $translator->getSetting('url')) {
      $url = $custom_url;
    }
    else {
      $url = $translator->getSetting('use_sandbox') ? $this->sandboxURL : $this->apiURL;
    }
    $url .= $path;

    // Build query array to pass to OHT service
    $query = array(
      'account_id' => $translator->getSetting('api_account_id'),
      'secret_key' => $translator->getSetting('api_secret_key'),
    ) + $query;

    // If we will send GET request - add params to the URL
    if ($method == 'GET') {
      $url = url($url, array('query' => $query));
    }
    // otherwise - configure $options array
    elseif ($method == 'POST') {
      $options += array(
        'headers' => array('Content-Type' => 'application/x-www-form-urlencoded'),
        'method' => 'POST',
        'data' => drupal_http_build_query($query),
      );
    }


    return drupal_http_request($url, $options);
  }

  /**
   * Prepares data to be send to OHT service and returns XML string.
   * @param TMGMTJob $job
   * @return string
   */
  protected function prepareDataForSending(TMGMTJob $job) {
    $data = array_filter(tmgmt_flatten_data($job->getData()), '_tmgmt_filter_data');
    $items = '';
    foreach($data as $key => $value) {
      $items .= str_replace(array('@key', '@text'), array($key, $value['#text']), '<item key="@key"><text type="text/html"><![CDATA[@text]]></text></item>');
    }

    return '<items>' . $items .'</items>';
  }

  /**
   * Parses received translation from OHT and returns unflatted data
   * @param $data string Base64 encode data, receveived from OHT
   * @return array Unflatted data
   */
  protected function parseTranslationData($data) {
    $data = base64_decode($data);
    $items = simplexml_load_string($data);
    $data = array();

    foreach($items->item as $item) {
      $key = (string) $item['key'];
      $data[$key]['#text'] = (string)$item->text;
    }

    return tmgmt_unflatten_data($data);
  }
}
