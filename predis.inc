<?php

/**
 * @file
 * Redis cache backend using Predis PHP library as Redis client.
 * 
 * Please read README.Predis.txt
 */

// Define Predis base path if not already set, and if we need to set the
// autoloader by ourself. This will ensure no crash. Best way would have
// been that Drupal ships a PSR-0 autoloader, in which we could manually
// add our library path.
if (!class_exists('Predis\Client')) {

  // Define Predis library base path if not already done.
  if (!defined('PREDIS_BASE_PATH')) {
    /**
     * Predis library base path.
     */
    define('PREDIS_BASE_PATH', dirname(__FILE__) . '/predis/lib/');
  }

  // Register a simple autoloader for Predis library. Since the Predis
  // library is PHP 5.3 only, we can afford doing closures safely.
  spl_autoload_register(function($class) {
    $file = PREDIS_BASE_PATH . strtr($class, '\\', '/') . '.php';
    if (file_exists($file)) {
      require $file;
      return TRUE;
    }
  });
}

/**
 * Predis cache backend.
 */
class RedisPredisCache implements DrupalCacheInterface {
  /**
   * @var Predis\Client
   */
  protected static $_client;

  protected static function _init() {
    if (isset(self::$_client)) {
      return;
    }

    global $conf;

    if (isset($conf['redis_cache_uri'])) {
      self::$_client = new Predis\Client($conf['redis_cache_uri']);
    }
    else {
      // No parameters given, attempt a connection on localhost.
      self::$_client = new Predis\Client();
    }
  }

  /**
   * @var string
   */
  protected $_bin;

  function __construct($bin) {
    // Force static init if not already done by another instance.
    self::_init();
    $this->_bin = $bin;
  }

  protected function _buildKey($cid) {
    // FIXME: Handle site.
    return $this->_bin . ':' . $cid;
  }

  function get($cid) {
    $client     = self::$_client;
    $key        = $this->_buildKey($cid);

    $serialized = $client->get($key . ':serialized');

    if (!isset($serialized)) {
      return FALSE;
    }

    $cached          = new stdClass;
    $cached->data    = $client->get($key . ':data');
    $cached->expires = 0; // FIXME: Redis does not seem to allow us to fetch
                          // expire value. The only solution would be to create
                          // a new key. Who on earth need this value anyway?

    if ($serialized) {
      $cached->data  = unserialize($cached->data);
    }

    return $cached;
  }

  function getMultiple(&$cids) {
    $client = self::$_client;

    $ret = $keys = $exclude = array();

    foreach ($cids as $cid) {
      $key       = $this->_buildKey($cid);
      $keys[]    = $key . ':data';
      $keys[]    = $key . ':serialized';
    }

    $result = $client->mget($keys);

    $index = 0;
    foreach ($cids as $cid) {
      $serialized = $result[$index + 1];

      if (!$serialized) {
        $exclude[$cid] = TRUE;

        continue;
      }

      $cached          = new stdClass;
      $cached->data    = $result[$index];
      $cached->expires = 0; // FIXME: See comment in get() method.
  
      if ($serialized) {
        $cached->data  = unserialize($cached->data);
      }

      $ret[$cid] = $cached;
      $index += 2;
    }

    // WTF Drupal, we need to manually remove entries from &$cids.
    foreach ($cids as $index => $cid) {
      if (isset($exclude[$cid])) {
        unset($cids[$index]);
      }
    }

    return $ret;
  }

  function set($cid, $data, $expire = CACHE_PERMANENT) {
    $client = self::$_client;
    $key    = $this->_buildKey($cid);

    $client->pipeline(function($pipe) use ($key, $data, $expire) {

      if (isset($data) && !is_scalar($data)) {
        $serialize = TRUE;
        $data      = serialize($data);
      }
      else {
        $serialize = FALSE;
      }

      switch ($expire) {

        // FIXME: Handle CACHE_TEMPORARY correctly.
        case CACHE_TEMPORARY:
        case CACHE_PERMANENT:
          $pipe->set($key . ':data',        $data);
          $pipe->set($key . ':serialized' , $serialize);
          // We dont need the PERSIST command, since it's the default.
          break;

        default:
          $delay = $expire - time();
          $pipe->setex($key . ':data',       $delay, $data);
          $pipe->setex($key . ':serialized', $delay, $serialize);
      }
    });
  }

  function clear($cid = NULL, $wildcard = FALSE) {
    $client = self::$_client;
    $many   = FALSE;

    // Redis handles for us cache key expiration.
    if (!isset($cid)) {
      return;
    }

    if ('*' !== $cid && $wildcard) {
      $key  = $this->_buildKey('*' . $cid . '*');
      $many = TRUE;
    }
    else if ('*' === $cid) {
      $key  = $this->_buildKey($cid);
      $many = TRUE;
    }
    else {
      $key = $this->_buildKey($cid);
    }

    if ($many) {
      $keys = $client->keys($key);

      // Attempt to clear an empty array will raise exceptions.
      if (!empty($keys)) {
        $client->del($keys);
      }
    }
    else {
      $client->del(array(
        $key . ':data',
        $key . ':serialized',
      ));
    }
  }

  function isEmpty() {
    // FIXME: Todo.
  }
}
