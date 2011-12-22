<?php

/**
 * @file
 * Custom exception definitions
 */

/**
 * Common parent class for all services client module exceptions
 */
class ServicesClientException extends Exception {
  
}

class ServicesClientNonExistingAdapterException extends ServicesClientException {
  
}

class ServicesClientMissingAdapterMappingException extends ServicesClientException {
  
}