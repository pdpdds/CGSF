#!/usr/bin/env ruby

require 'fastdb'
require 'benchmark'

def returning(a)
  yield a
  a
end

N_NAMES=1000
NAMES=Array.new(N_NAMES)
for i in 0...N_NAMES
   NAMES[i] = rand(1000000007)
end

class Person < Struct.new(:name, :salary, :rating, :address)
  @fname_num = 0
  @lname_num = NAMES.size/2
  @sal = 10000
  @rating = 1
  def self.makeup_something
    returning(new("#{NAMES[@fname_num]} #{NAMES[@lname_num]}", @sal % 53323, @rating % 10)) do
      @fname_num += 3
      @lname_num += 6
      @fname_num -= NAMES.size if @fname_num >= NAMES.size
      @lname_num -= NAMES.size if @lname_num >= NAMES.size
      @sal += 4242
      @rating += 13
    end
  end
end

con = FastDB::Connection.new
Benchmark.bm do |x|
  x.report("connecting") { con.open("localhost", 6100) }
  x.report("inserting") do
    7.times { con.insert(Person.makeup_something) }
    con.commit()
  end
  x.report("scanning by rating") do
    stmt = con.createStatement("select * from Person where rating > %rating order by name")
    stmt["rating"] = 5
    cursor = stmt.fetch()
  end

  x.report("loading anything?") do
    stmt = con.createStatement("select * from Person")
    cursor = stmt.fetch(true)
    cursor.removeAll()
    stmt.close()
  end
end

con.close()
