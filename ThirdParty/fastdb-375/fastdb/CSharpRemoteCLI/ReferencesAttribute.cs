using System;

namespace CSharpCLI
{
    
    /// <summary>
    /// Class representing references attribute. This attribute should be used to
    /// to specify name of the referenced table for reference or array of reference fields
    /// </summary>
    [AttributeUsage(AttributeTargets.Field)]
    public class ReferencesAttribute : Attribute 
    {
        /// <summary>
        /// Specify referenced table and inverse reference field
        /// </summary>
        /// <param name="tableName">name of referenced table</param>
        /// <param name="fieldName">name of inverse reference field in referenced table</param>
        ///
        public ReferencesAttribute(String tableName, String fieldName) {
             refTableName = tableName;
             inverseRefName = fieldName;
        }
        /// <summary>
        /// Specify referenced table 
        /// </summary>
        /// <param name="tableName">name of referenced table</param>
        ///
        public ReferencesAttribute(String tableName) {
             refTableName = tableName;
        }

        internal string refTableName;
        internal string inverseRefName;
    }
}
