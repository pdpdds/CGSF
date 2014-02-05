using System;
using FastDbNet;
using System.Diagnostics;

namespace FastDbNetTest {
  public class Test {
    /// <summary>
    /// The main entry point for the application.
    /// </summary>
    [STAThread]
    static void Main(string[] args)
    {
      int  rc, i, n;
      uint oid = 0;
      int  nRecords = 200000;

      FastDbConnection connection = new FastDbConnection("test");
      connection.InitDbSize       = FastDbConnection.DefaultInitDatabaseSize*2;
      connection.InitIdxSize      = FastDbConnection.DefaultInitIndexSize*2;
      connection.ExtensionQuantum = FastDbConnection.DefaultExtensionQuantum*2;
      connection.Open();

      FastDbFields fields = new FastDbFields();
      fields.Add("int_field", CLI.FieldType.cli_int8,   CLI.FieldFlags.cli_hashed);
      fields.Add("str_field", CLI.FieldType.cli_asciiz, CLI.FieldFlags.cli_hashed);

      rc = connection.CreateTable("test", fields);
      if (rc < 0) Console.WriteLine("cli_create_table: {0}", CLI.CliErrorToStr(rc));

      FastDbCommand command = connection.CreateCommand("insert into test");
      command.Fields.Assign(fields);  // You could also do:  command.Describe();

      DateTime start = DateTime.Now;

      long key = 1999;

      for (i = 0; i < nRecords; i++) {
        key = (3141592621L * key + 2718281829L) % 1000000007L;
        command.Fields[0].asInt64 = key;
        command.Fields[1].asString = key.ToString();
        oid = command.Insert();
      }

      connection.Commit();
      Console.WriteLine("Elapsed time for inserting {0} records: {1:F2} (speed: {2:F2})", 
        nRecords, 
        (DateTime.Now - start).TotalSeconds, 
        nRecords / (DateTime.Now - start).TotalSeconds);

		  FastDbCommand command1 = connection.CreateCommand("select * from test where int_field = %key");
      FastDbCommand command2 = connection.CreateCommand("select * from test where str_field = %key");
      command1.Parameters.Add("key", CLI.FieldType.cli_int8);
      command2.Parameters.Add("key", CLI.FieldType.cli_asciiz);
      command1.Fields.Assign(fields);
      command2.Fields.Assign(fields);

      int rc1;
      int rc2;

      key = 1999;
      start = System.DateTime.Now;
      for (i = 0; i < nRecords; i++) {
        key = (3141592621L * key + 2718281829L) % 1000000007L;
        command1.Parameters[0].asInt64  = key;
        command2.Parameters[0].asString = key.ToString();
        rc1 = command1.Execute();
        rc2 = command2.Execute();
      }     
      System.Console.WriteLine("Elapsed time for performing {0} index searches {1:F2} (speed: {2:F2})", 
        nRecords * 2, (DateTime.Now - start).TotalSeconds, 
        nRecords * 2 / (DateTime.Now - start).TotalSeconds);

      command.SQL = "select * from test";

      start = System.DateTime.Now;

      n = command.Execute();
      for(i=0; i < n; i++) {
        command.Next();
      }

      Console.WriteLine(String.Format("Elapsed time for iteration through {0} records: {1:F2} (speed: {2:F2})", 
        nRecords, 
        (DateTime.Now - start).TotalSeconds, 
        nRecords / (DateTime.Now - start).TotalSeconds));

      start = System.DateTime.Now;
      /*
      key = 1999;
      for (i = 0; i < nRecords; i++) {
        key = (3141592621L * key + 2718281829L) % 1000000007L;
        Record rec = (Record) intIndex.Get(new Key(key));
        Record removed = (Record)intIndex.Remove(new Key(key));
        Debug.Assert(removed == rec);
        strIndex.Remove(new Key(System.Convert.ToString(key)), rec);
        rec.Deallocate();
      }*/
      command.Execute(true);
      command.Delete();
      connection.Commit();
      System.Console.WriteLine("Elapsed time for deleting {0} records: {1:F2} (speed: {2:F2})", 
        nRecords, 
        (DateTime.Now - start).TotalSeconds, 
        nRecords/(DateTime.Now - start).TotalSeconds);
    }
	}
}
