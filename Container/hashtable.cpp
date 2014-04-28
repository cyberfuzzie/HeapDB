
/**
  * Non-template HashTable utility function
  */

/**
 * @brief checkFlags tests whether the given flags are in the specified state
 * @param flags the flags value to test
 * @param pos flags which have to be set
 * @param neg flags which must not be set
 * @return bool
 */
bool checkFlags(unsigned int flags, unsigned int pos, unsigned int neg) {
    return ((flags & pos) == pos) && ((flags & neg) == 0);
}
