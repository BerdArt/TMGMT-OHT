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
  protected $apiURL = 'http://api.onehourtranslation.com/';
  protected $sandboxURL = 'http://sandbox.onehourtranslation.com/';
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
    'ro' => 'ru-ru',
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
  public function canTranslate(TMGMTJob $job) {
/*    $translator = $job->getTranslator();
    if ($this->isAvailable($translator)) {
      $targets = $this->getSupportedTargetLanguages($translator, $job->source_language);
      if (isset($targets[$job->target_language])) {
        return TRUE;
      }
    }
    return FALSE;*/
    return TRUE;
  }

  /**
   * Implements TMGMTTranslatorPluginControllerInterface::requestTranslation().
   *
   * Here we will acutally query source and get translations.
   */
  public function requestTranslation(TMGMTJob $job) {
    // Pull the source data array through the job and flatten it.
    $data = $this->prepareDataForSending($job);
    dsm($job, 'Job');
    dsm($job->getSourceData(), 'Data-raw');
    dsm($data, 'Data');
/*    $translations = $this->generateMyGengoTranslationJobArray($job, $job->settings['quality']);

    $response = $this->requestFromGengo($job->getTranslator(), 'translate/jobs', 'POST', array(
      'jobs' => $translations,
      'process' => 1,
      'as_group' => 1
      ));

    if (!empty($response)) {
      $job->submitted('Job has been submitted.');
    }
    else {
      // @todo: Get a error message from mygengo.
      $job->rejected('Job has been rejected.');
    }*/
  }

  /**
   * Receives and stores a translation returned by myGengo.
   */
  public function receiveTranslation(TMGMTJob $job, $keys, $data) {
    // @todo Clean this up once sandbox and production return the same thing.
/*    if ($job->getTranslator()->getSetting('use_sandbox')) {
      $translation = $data->body_src;
    }
    else {
      $translation = $data->body_tgt;
    }
    $job->addTranslatedData(array('#text' => $data->body_src), $keys);*/
  }

  /**
   * Returns list of languages supported by OHT service
   * @param TMGMTTranslator $translator
   * @param $source_language
   * @return array
   */
  public function getSupportedTargetLanguages(TMGMTTranslator $translator, $source_language) {
    return drupal_map_assoc(array_values($this->supportedLanguages));
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
  protected function doRequest(TMGMTTranslator $translator, $path, array $query = array(), array $options = array()) {
    if ($custom_url = $translator->getSetting('url')) {
      $url = $custom_url;
    }
    else {
      $url = $translator->getSetting('use_sandbox') ? $this->sandboxURL : $this->apiURL;
    }
    $url .= $path;
    // Add the account_id and secret_key to the query arguments and build the query string.
    $query = array(
      'account_id' => $translator->getSetting('api_account_id'),
      'secret_key' => $translator->getSetting('api_secret_key'),
    ) + $query;
    $url = url($url, array('query' => $query));
    return drupal_http_request($url, $options);
  }

  /**
   * Prepares data to be send to OHT service and returns XML string.
   * @param TMGMTJob $job
   * @return string
   */
  protected function prepareDataForSending(TMGMTJob $job) {
    $data = array_filter(tmgmt_flatten_data($job->getSourceData()), '_tmgmt_filter_data');
    $items = '';
    foreach($data as $key => $value) {
      $items .= str_replace(array('@key', '@text'), array($key, $value['#text']), '<item key="@key"><text type="text/html"><![CDATA[@text]]></text></item>');
    }
    return '<items>' . $items .'</items>';
  }
}
