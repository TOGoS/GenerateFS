#!/usr/bin/ruby

require 'fileutils';

def phail( text )
  STDERR.puts text
  exit 1
end

def test_write( file, text )
  #STDERR.puts "open in truncate mode..."
  fh = open(file,'w')
  fh.write text
  fh.close
  if File.size(file) != text.length
    phail "Expected size of written file to be #{text.length}, but was #{File.size(file)}"
  end
  readtext = File.read(file)
  if readtext != text
    phail "Expected text read from file to be #{text.inspect}, but was #{text.inspect}"
  end
end

def test_append( file, text )
  unless File.exist? file
    phail "Expected '#{file}' to exist before appending, but it does not."
  end
  origsize = File.size( file )
  #STDERR.puts "open in append mode..."
  fh = open(file,'a')
  fh.write text
  fh.close
  expectednewsize = origsize + text.length
  if File.size(file) != expectednewsize
    phail "Expected size of written file to be #{expectednewsize} (#{origsize} + #{text.length}), but was #{File.size(file)}"
  end
  readtext = File.read(file)
  appendedtext = readtext[origsize..-1]
  if appendedtext != text
    phail "Expected end of text read from file to be #{text.inspect}, but was #{appendedtext.inspect}"
  end
end




Thread.abort_on_exception = true

debug = false
shell = nil
moar_debug = false
filesystem = nil
usetestserver = false
useextserver = false
mountpoint = nil
host = '127.0.0.1'
port = 23823

args = $*.clone
while arg = args.shift
  case arg
  when '-debug'
    debug = true
  when '-moar-debug'
    moar_debug = true
  when '-host'
    host = args.shift
  when '-port'
    port = args.shift.to_i
  when '-use-testserver'
    usetestserver = true
  when '-use-extserver'
    useextserver = true
  when '-fs'
    filesystem = args.shift
  when '-shell'
    shell = args.shift
  when /^[^-]/
    mountpoint = arg
  else
    STDERR.puts "#{$0}: Unrecognised argument: #{arg}"
    exit 1
  end
end

unless mountpoint
  STDERR.puts "No mountpoint specified (specify with bare argument)"
  exit 1
end

unless filesystem
  STDERR.puts "No filesystem specified (-fs <fuse-executable>)"
  exit 1
end

fuseargs = []
if usetestserver
  fuseargs << "--server-host=#{host}"
  fuseargs << "--server-port=#{port}"
end
fuseargs << '-d' if debug
fuseargs << '--moar-debug' if moar_debug
fuseargs << '-f'
fuseargs << mountpoint

begin
  if usetestserver
    serverpid = fork do
      require 'TOGoS/GeneratorFS/TestServer';
      server = TOGoS::GeneratorFS::TestServer.new
      server.host = host
      server.port = port
      server.debug = debug
      #STDERR.puts "Starting testserver..."
      server.run
    end
  else
    serverpid = nil
  end

  #STDERR.puts "Starting fs..."
  cpid = fork do
    exec filesystem,*fuseargs
  end
  sleep 0.25 # give it time to start...
 
  if shell
    if shell
      system shell
    end
  else
    barfile = "#{mountpoint}/bartest.txt"
    unless useextserver
      if File.exist? barfile
        phail "#{barfile} already exists, but was expected not to."
      end
    end
    
    # Test create new file
    test_write( barfile, "Hello, world!" )
    
    # Test rewrite the file
    test_write( barfile, "Hello, world!" )
    
    # Test appending to the file
    test_append( barfile, "Goop Mah Scoop" )
  end
ensure
  if serverpid
    #STDERR.puts "Stopping server..."
    Process.kill "TERM", serverpid
    Process.waitpid serverpid
  end
  if cpid
    #STDERR.puts "Stopping fs..."
    Process.kill "TERM", cpid
    Process.waitpid cpid
  end
end
  
