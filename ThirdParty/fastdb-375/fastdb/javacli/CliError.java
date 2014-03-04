package javacli;

/**
 * Exception thrown by Gigabase CLI implementation.
 * It is derived from RuntimeException class so programmer should not delare
 * this exception in throws part.
 */
public class CliError extends RuntimeException { 
    /**
     * Constructor of the exception object.
     * @param msg message describing the reason of the fault
     */
    public CliError(String msg) { 
        super(msg);
    }
}
