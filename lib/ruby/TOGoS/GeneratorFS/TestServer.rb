#!/usr/bin/ruby

require 'timeout'
require 'socket'
require 'fileutils'

Thread.abort_on_exception = true

module TOGoS ; module GeneratorFS
  def self.detokenize( tokens )
    return tokens.collect { |a|
      a = a.to_s
      a =~ /[\s"\\]/ ? a.inspect : a
    }.join(' ')
  end
  
  class ClientSocket
    def initialize( sock, debug=false )
      @sock = sock
      @debug = debug
    end
    
    def read_command
      leine = @sock.gets
      leine.strip!
      loine = []
      leine.scan /"((?:[^"]|\\[\\"])+)"|(\S+)/ do loine << ($1 || $2) end
      return loine
    end
    
    def write_line( cmd, *args )
      line = GeneratorFS.detokenize( [cmd, *args] )
      STDERR.puts( "TestServer: writing line: #{line}" ) if @debug
      @sock.puts line
    end
    
    def write_stat( size, mode )
      write_line 'OK-STAT', size, mode
    end
    
    def write_client_error( *text )
      write_line 'CLIENT-ERROR', *text
    end

    def write_server_error( *text )
      write_line 'SERVER-ERROR', *text
    end
    
    def write_does_not_exist
      write_line 'DOES-NOT-EXIST'
    end
    
    def write_permission_denied
      write_line 'PERMISSION-DENIED'
    end
    
    def write_invalid_operation
      write_line 'INVALID-OPERATION'
    end
    
    def write_dir_entry( name, size, mode )
      write_line 'DIR-ENTRY', name, size, mode
    end
    
    def write_ok_truncated
      write_line 'OK-TRUNCATED'
    end
    
    def write_ok_created
      write_line 'OK-CREATED'
    end
    
    def write_ok_alias( dest )
      write_line 'OK-ALIAS', dest
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
    def initialize
      @host = '127.0.0.1'
      @port = 23823
      @debug = false
    end
    
    attr_accessor :host, :port, :debug
    
    def run
      serv = TCPServer.new(@host,@port)
      STDERR.puts "TestServer: Listening for connections on #{@port}" if @debug
      while s = serv.accept
        STDERR.puts "TestServer: Got connection from #{s.peeraddr[3]}" if @debug
        Thread.new( s ) { |sock|
          cs = ClientSocket.new(sock, @debug)
          begin
            loine = cs.read_command
            STDERR.puts "TestServer: received "+GeneratorFS.detokenize(loine) if @debug
            case loine[0]
            when 'GET-STAT'
              case loine[1]
              when nil
                cs.write_client_error "Missing argument to GET-STAT"
              when '/'
                cs.write_stat   0, '0040755'
              when '/test1.txt'
                cs.write_stat 128, '0100644'
              when '/subdir'
                cs.write_stat   0, '0040755'
              when '/subdir/test2.txt'
                cs.write_stat 700, '0100700'
              when '/secret.txt'
                cs.write_permission_denied
              when '/server-error'
                cs.write_server_error
              else
                if File.exist? 'temp'+loine[1]
                  stat = File.stat('temp'+loine[1])
                  cs.write_stat stat.size, sprintf("0%o",stat.mode)
                else
                  cs.write_does_not_exist
                end
              end
            when 'OPEN-READ'
              case loine[1]
              when nil
                cs.write_client_error "Missing argument to OPEN-READ"
              when '/'
                cs.write_invalid_operation
              when '/test1.txt'
                FileUtils.mkdir_p('testdata')
                open( 'testdata/test1.txt', 'w' ) do |s|
                  s.write( ("test1 test1 test1\n"*8)[0..127]+"\n" )
                end
                cs.write_ok_alias 'testdata/test1.txt'
              when '/subdir'
                cs.write_invalid_operation
              when '/subdir/test2.txt'
                FileUtils.mkdir_p('testdata')
                open( 'testdata/test2.txt', 'w' ) do |s|
                  s.write( ("test2 test2 test2 test2\n"*20)[0..699]+"\n" )
                end
                cs.write_ok_alias 'testdata/test2.txt'
              when '/secret.txt'
                cs.write_permission_denied
              when '/server-error'
                cs.write_server_error
              else
                if File.exist? 'temp'+loine[1]
                  cs.write_ok_alias 'temp'+loine[1]
                else
                  cs.write_does_not_exist
                end
              end
            when 'CLOSE-READ'
              cs.write_line 'OK-CLOSED'
            when 'TRUNCATE'
              realfile = 'temp'+loine[1]
              open( realfile, 'w' ) {}
              cs.write_ok_truncated
            #when 'CREATE'
            #  fn = loine[1]
            #  open( 'temp'+loine[1], 'w' ) {}
            #  cs.write_ok_created
            when 'CREATE+OPEN-WRITE', 'OPEN-WRITE'
              realfile = 'temp'+loine[1]
              unless File.exist? realfile
              # Need to create it for them....
                open( realfile, 'w' ) {}
              end
              cs.write_ok_alias realfile
            when 'CLOSE-WRITE'
              if File.exist? loine[2]
                cs.write_line 'OK-CLOSED'
              else
                cs.write_does_not_exist
              end
            when 'READ-DIR'
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
              when '/secret.txt'
                cs.write_invalid_operation
              when '/server-error'
                cs.write_server_error
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
  timeout = nil
  ts = TOGoS::GeneratorFS::TestServer.new
  
  args = $*.clone
  while arg = args.shift
    case arg
    when '-timeout'
      timeout = args.shift.to_f
    when '-port'
      ts.port = args.shift.to_i
    when '-debug'
      ts.debug = true
    else
      STDERR.puts "Unrecognised arg #{arg}"
      exit 1
    end
  end
  
  FileUtils.rm_rf 'temp'
  FileUtils.mkdir_p 'temp'
  if timeout
    begin
      timeout timeout do
        ts.run
      end
    rescue Timeout::Error
      STDERR.puts "#{$0}: Quitting due to timeout"
    end
  else
    ts.run
  end
end
