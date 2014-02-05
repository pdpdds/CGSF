package jnicli;

import java.util.Enumeration;

/**
 * Iterator through result set returned by Database.select.
 * There are two types of cursors: incremental and prefetched.
 * Incremental cursor fetch records on demand (one record at each iteration).
 * And prefetched cursor loads all selected records. 
 * Prefetched cursor is used in case of remote database connections and when AUTOCOMMIT flag is set in select
 */
public interface Cursor { // extends Enumeration { // if Cursor is derived from enumeration, then Hessian tries tp deserialize it as Vector
    /**
     * Tests if this enumeration contains more elements.
     *
     * @return  <code>true</code> if and only if this enumeration object
     *           contains at least one more element to provide;
     *          <code>false</code> otherwise.
     */
    boolean hasMoreElements();

    /**
     * Returns the next element of this enumeration if this enumeration
     * object has at least one more element to provide.
     *
     * @return     the next element of this enumeration.
     * @exception  NoSuchElementException  if no more elements exist.
     */
    Object nextElement();

    /**
     * Get OID of the current object. This method should be used only after
     * <code>Enumeration.nextElement()</code> method and its result is OID of the object returned
     * by <code>nextElement()</code>.
     * @exception  java.util.NoSuchElementException if there is no current element
     */
    long getOid();

    /**
     * Update current object. This method should be invoked after <code>Enumeration.nextElement()</code>
     * and store updated object, returned by  <code>nextElement()</code>.
     * This method is not supported for prefetched cursor (which is returned for remote database or
     * when AUTOCOMMIT flag is set in select). In this case you should use Database.update method
     * specifying OID of updated object. This OID can be obtained using Cursor.getOid() method.
     * @exception  java.util.NoSuchElementException if there is no current element
     */
    void update();

    /**
     * Get number of selected records
     */
    int size();
}