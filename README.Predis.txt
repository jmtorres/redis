Predis cache backend
====================

This client, for now, is only able to use the Predis PHP library.

The Predis library requires PHP 5.3 minimum. If your hosted environment does
not ships with at least PHP 5.3, please do not use this cache backend.

This code is ALPHA code. This means: DO NOT USE IT IN PRODUCTION. Not until
I don't ship any BETA release as a full Drupal.org module.

Please consider using an OPCode cache such as APC. Predis is a good and fully
featured API, the cost is that the code is a lot more than a single file in
opposition to some other backends such as the APC one.

Get Predis
----------

You can download this library at:
  https://github.com/nrk/predis

This file explains how to install the Predis library and the Drupal cache
backend. If you are an advanced Drupal integrator, please consider the fact
that you can easily change all the pathes. Pathes used in this file are
likely to be default for non advanced users.

Configuration (no sharing)
--------------------------

Once done, you either have to clone it into:
  modules/predis/predis

So that you have the following directory tree:
  sites/all/modules/redis_cache/predis.inc
  sites/all/modules/redis_cache/predis/lib/Predis # Where the PHP code stands

Configuration (sharing)
-----------------------

Or, any other place in order to share it:
For example, into your libraries folder, in order to get:
  sites/all/libraries/predis/lib

If you choose this solution, you have to alter a bit your $conf array into
the settings.php file as this:
  define('PREDIS_BASE_PATH', DRUPAL_ROOT . '/sites/all/libraries/predis/lib/');

Tell Drupal to use it
---------------------

Usual cache backend configuration, as follows, to add into your settings.php
file like any other backend:
  $conf['cache_backends'][]            = 'sites/all/modules/redis_cache/predis.inc';
  $conf['cache_class_cache']           = 'RedisPredisCache';
  $conf['cache_class_cache_menu']      = 'RedisPredisCache';
  $conf['cache_class_cache_bootstrap'] = 'RedisPredisCache';
  // ... Any other bins.

Additionnaly, if your Redis server is remote, you can add the remote connection
string this way (local instance doesn't need connection string, Predis will
attempt a connection on localhost per default):
  $conf['redis_cache_uri'] = 'tcp://1.2.3.4:1234';

Or this way:
  $conf['redis_cache_uri'] = array(
    'scheme' => 'tcp',
    'host'   => '1.2.3.4',
    'port'   => 1234',
  );

That's it, refresh your Drupal frontpage and pray.

Advanced configuration (PHP expert)
-----------------------------------

Best solution is, whatever is the place where you put the Predis library, that
you set up a fully working autoloader able to use it.
