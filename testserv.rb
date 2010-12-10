#!/usr/bin/ruby

require 'socket'
require 'fileutils'

Thread.abort_on_exception = true

module TOGoS ; module GeneratorFS
  class ClientSocket
    def initialize( sock )
      @sock = sock
    end
    
    def read_command
      leine = @sock.gets
      leine.strip!
      loine = []
      leine.scan /"((?:[^"]|\\[\\"])+)"|(\S+)/ do loine << ($1 || $2) end
      return loine
    end
    
    def write_line( *args )
      @sock.puts args.collect { |a|
        a = a.to_s
        a =~ /[\s"\\]/ ? a.inspect : a
      }.join(' ')
    end
    
    def write_stat( size, mode )
      write_line 'OK-STAT', size, mode
    end
    
    def write_client_error( text )
      write_line 'CLIENT-ERROR', text
    end
    
    def write_does_not_exist
      write_line 'DOES-NOT-EXIST'
    end
    
    def write_invalid_operation
      write_line 'INVALID-OPERATION'
    end
    
    def write_dir_entry( name, size, mode )
      write_line 'DIR-ENTRY', name, size, mode
    end
    
    def write_dir_list
      write_line 'OK-DIR-LIST'
      yield
      write_line 'END-DIR-LIST'
    end
    
    def close
      @sock.close
    end
  end
  
  class TestServer
    def run
      serv = TCPServer.new('127.0.0.1',23823)
      while s = serv.accept
        Thread.new( s ) { |sock|
          cs = ClientSocket.new(sock) 
          begin
            loine = cs.read_command
            case loine[0]
            when 'GET-STAT'
              case loine[1]
              when nil
                cs.write_client_error "Missing argument to GET-STAT"
              when '/'
                cs.write_stat   0, '0040755'
              when '/test1.txt'
                cs.write_stat 100, '0100644'
              when '/subdir'
                cs.write_stat   0, '0040755'
              when '/subdir/test2.txt'
                cs.write_stat 100, '0100700'
              else
                cs.write_does_not_exist
              end
            when 'OPEN-READ', 'OPEN-WRITE'
              case loine[1]
              when nil
                cs.write_client_error "Missing argument to OPEN-READ"
              when '/'
                cs.write_invalid_operation
              when '/test1.txt'
                cs.write_ok_alias 'test-data/test1.txt'
              when '/subdir'
                cs.write_invalid_operation
              when '/subdir/test2.txt'
                cs.write_ok_alias 'test-data/test2.txt'
              else
                cs.write_does_not_exist
              end
            when 'GET-DIR'
              case loine[1]
              when nil
                cs.write_client_error "Missing argument to GET-DIR"
              when '/'
                cs.write_dir_list {
                  cs.write_dir_entry 'test1.txt', 128, '0100644'
                  cs.write_dir_entry 'subdir',      0, '0040755'
                }
              when '/test1.txt'
                cs.write_invalid_operation
              when '/subdir'
                cs.write_dir_list {
                  cs.write_dir_entry 'test2.txt', 700, '0100600'
                }
              when '/subdir/test1.txt'
                cs.write_invalid_operation
              else
                cs.write_does_not_exist
              end
            else
              cs.write_client_error "Unrecognised command: #{loine[0].inspect}"
            end
          ensure
            cs.close
          end
        }
      end
    end
  end
end ; end

if __FILE__ == $0
  TOGoS::GeneratorFS::TestServer.new.run
end
