namespace CSharpCLI { 

using System;

/// <summary>
/// Exception thrown by Gigabase CLI implementation.
/// It is derived from RuntimeException class so programmer should not delare
/// this exception in throws part.
/// </summary>
public class CliError : Exception { 
    ///<summary>
    /// Constructor of the exception object.
    /// </summary>
    /// <param name="msg">message describing the reason of the fault</param>
    ///
    public CliError(string msg) : base(msg) {}
}

}
