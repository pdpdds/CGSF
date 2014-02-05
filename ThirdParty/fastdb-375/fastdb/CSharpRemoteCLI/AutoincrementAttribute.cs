using System;

namespace CSharpCLI
{
    
    /// <summary>
    /// Class representing autoincrement attribute
    /// </summary>
    [AttributeUsage(AttributeTargets.Field)]
    public class AutoincrementAttribute : Attribute 
    {
        /// <summary>
        /// Default constructor
        /// </summary>
        public AutoincrementAttribute() {}
    }
}
