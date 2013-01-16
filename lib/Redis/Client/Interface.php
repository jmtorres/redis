<?php

/**
 * Client proxy, client handling class tied to the bare mininum.
 */
interface Redis_Client_Interface {
  /**
   * Get the connected client instance.
   *
   * @param string|array $host
   *   Server hostname. This variable can be an array of servers, case in which
   *   all other parameters will be set to null. If this is an array, each value
   *   must be an array containing one server information, corresponding to the
   *   $conf['redis_client_servers'] variable structure.
   * @param string $port
   *   Server port
   * @param string $base
   *   Server database
   * @param string $password
   *   Server password
   *
   * @return mixed
   *   Real client depends from the library behind.
   */
  public function getClient($host = NULL, $port = NULL, $base = NULL, $password = NULL);

  /**
   * Get underlaying library name used.
   * 
   * This can be useful for contribution code that may work with only some of
   * the provided clients.
   * 
   * @return string
   */
  public function getName();
}
