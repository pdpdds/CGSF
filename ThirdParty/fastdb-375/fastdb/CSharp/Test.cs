using System;
using FastDbNet;

namespace FastDbNetTest
{
	public class Test
	{
    /// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main(string[] args)
		{
      int  rc;
      uint oid = 0;
      int  nRecords = 3;

      FastDbConnection connection = new FastDbConnection("test");
      connection.InitDbSize       = FastDbConnection.DefaultInitDatabaseSize*2;
      connection.InitIdxSize      = FastDbConnection.DefaultInitIndexSize*2;
      connection.ExtensionQuantum = FastDbConnection.DefaultExtensionQuantum*2;
      connection.Open();

      FastDbFields fields = new FastDbFields();
      fields.Add("int_field", CLI.FieldType.cli_int8,   CLI.FieldFlags.cli_hashed);
      fields.Add("str_field", CLI.FieldType.cli_asciiz, CLI.FieldFlags.cli_indexed);

      rc = connection.CreateTable("test", fields);
      if (rc < 0) Console.WriteLine("cli_create_table: {0}", CLI.CliErrorToStr(rc));

      FastDbCommand command = connection.CreateCommand("insert into test");
      command.Fields.Assign(fields);  // You could also do:  command.Describe();

      DateTime time = DateTime.Now;

      long key = 1999;

      for (int i = 0; i < nRecords; i++)
      {
        key = (3141592621L * key + 2718281829L) % 1000000007L;
        command.Fields[0].asInt64 = key;
        command.Fields[1].asString = key.ToString();
        oid = command.Insert();
      }

      connection.Commit();
      Console.WriteLine(String.Format("Elapsed time for inserting {0} records: {1} (speed: {2:F2})", nRecords, DateTime.Now - time, nRecords*1000 / (DateTime.Now - time).TotalMilliseconds));

      command = connection.CreateCommand("insert into persons");
      
      command.Fields.Add("name",          CLI.FieldType.cli_asciiz, CLI.FieldFlags.cli_indexed);
      command.Fields.Add("salary",        CLI.FieldType.cli_int8);
      command.Fields.Add("address",       CLI.FieldType.cli_asciiz);
      command.Fields.Add("weight",        CLI.FieldType.cli_real8);
      command.Fields.Add("subordinates",  CLI.FieldType.cli_array_of_oid, CLI.FieldFlags.cli_noindex, "persons");

      rc = connection.CreateTable("persons", command.Fields);
      if (rc < 0)
        Console.WriteLine("cli_create_table: {0}", rc);

      Console.WriteLine("Name\tType\tSize\tCapacity\tFlags");
      for (int i=0; i < command.Fields.Count; ++i) 
      {
        FastDbField f = command.Fields[i];
        Console.WriteLine("{0}\t{1}\t{2}\t{3}\t{4}", 
          f.Name, f.Type, f.Size, f.Capacity, f.Flags);
      }
      
      connection.Commit();

      time = DateTime.Now;

      for (int i=0; i < nRecords; ++i) 
      {
        int n = 0;
        command.Fields[n++].asString = String.Format("Name{0}", i+1);
        command.Fields[n++].asInt64  = (i+1) * 1000;
        command.Fields[n++].asString = String.Format("Address {0}", 100-i);
        command.Fields[n++].asDouble = (i+1) * 100;
        command.Fields[n].ArraySize = (i==0) ? 0 : 1;
        if (i > 0)
          command.Fields[n].SetArrayValue(0, oid);

        oid = command.Insert();
      }
      connection.Commit();

      Console.WriteLine("Elapsed time for inserting {0} records: {1} (speed: {2:F2})", nRecords, DateTime.Now - time, nRecords*1000 / (DateTime.Now - time).TotalMilliseconds);

      command = connection.CreateCommand("select * from persons");
      //command.Execute();

      DumpRecords(command);
      connection.Commit();

      command = connection.CreateCommand(@"select * from persons where length(subordinates) < %subordinates and salary > %salary");
      command.Parameters.Add("subordinates", CLI.FieldType.cli_int4);
      command.Parameters.Add("salary",       CLI.FieldType.cli_int8);

      command.Parameters["subordinates"].asInteger = 1;
      command.Parameters["salary"].asInteger       = 100;

      //command.Execute();
      DumpRecords(command);
		}

    public static void DumpRecords(FastDbCommand command) {
      Console.WriteLine();
      Console.WriteLine("---SQL: \"{0}\"---", command.SQL);
      command.Describe();  // Ensure that field descriptions are fetched from the database.
      foreach (FastDbField f in command.Fields) 
        Console.Write("{0}\t", f.Name);
      Console.WriteLine();
      foreach(FastDbCommand.Cursor cur in command.Select()) {
        foreach (FastDbField f in cur.Fields) 
          Console.Write("{0}\t", f.asString);
        Console.WriteLine();
      }
    }
	}
}
