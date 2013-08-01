require 'find'
require 'fileutils'
require 'nokogiri'
require 'pathname'
require 'uuidtools'

class Module
  attr_accessor :name, :absolute_path, :sources, :headers, :assets, :forms, :source_settings, :project
  
  def project
    @project
  end
  
  def name=(n)
    @name = n
  end
  
  def name
    @name
  end
  
  def sources
    @sources
  end
  
  def headers
    @headers
  end
  
  def assets
    @assets
  end
  
  def forms
    @forms
  end
  
  def absolute_path
    @absolute_path
  end
  
  def source_settings
    @source_settings
  end
  
  def source_settings=(ss)
    @source_settings = ss
  end
  
  def source_files(pattern)
    @sources += Dir.glob("#{self.absolute_path}/#{pattern}")
  end
  
  def header_files(pattern)
    @headers += Dir.glob("#{self.absolute_path}/#{pattern}")
  end
  
  def asset_files(pattern)
    @assets += Dir.glob("#{self.absolute_path}/#{pattern}")
  end
  
  def form_files(pattern)
    @forms += Dir.glob("#{self.absolute_path}/#{pattern}")
  end
  
  def platform(p)
    if p.kind_of?(Array)
      yield if p.include?(self.project.prop(:platform).downcase.to_sym)
    elsif self.project.prop(:platform).downcase.to_sym == p
      yield
    end
  end
  
  def initialize(path, proj)
    @project = proj
    @absolute_path = path
    @sources = Array.new
    @headers = Array.new
    @assets = Array.new
    @forms = Array.new
    @source_settings = nil
  end
end

class Project
  
  attr_accessor :root_dir, :modules, :out_dir, :project_dir, :project_name, :props, :no_stub, :stubs
  
  def initialize(root_dir)
    @modules = Array.new
    @no_stub = false
    @root_dir = root_dir
    @props = {}
  end
  
  def stubs
    @stubs
  end
  
  def modules
    @modules
  end

  def root_dir
    @root_dir
  end
 
  def out_dir
    self.prop :out_dir
  end
  
  def project_dir
    @project_dir
  end
  
  def project_name
    @project_name
  end
  
  def props=(properties)
    @props = properties
  end
  
  def props
    @props
  end
  
  def prop(pname)
    v = @props[pname.to_sym]
    v = @props[pname] if v.nil?
    v
  end
  
  def no_stub
    @no_stub
  end
  
  def no_stub=(ns)
    @no_stub = ns
  end
  
  def find_module_by_name(module_name)
    @modules.each do |md|
      return md if md.name == module_name
    end
    nil
  end
  
  def has_module?(module_name)
    find_module_by_name(module_name) != nil
  end
  
  def add_module(mod)
    self.modules << mod
  end
  
  def parse_projfile(path)
    string = File.open(path, 'r:utf-8')  { |f| f.read }

    if string.respond_to?(:encoding) && string.encoding.name != "UTF-8"
      string.encode!('UTF-8')
    end

    begin
      self.instance_eval(string)
    rescue Exception => e
      puts "Invalid `#{File.basename(path)}` projfile: #{e.message}"
      puts e.backtrace
    end
  end
  
  def use_module(pattern)
    
    Dir.glob("#{self.root_dir}/#{pattern}").each do |path|
      puts "Modfile at #{path}"
      dir = File.dirname path
      md = Module.new(dir, self)
      md.name = dir

      string = File.open(path, 'r:utf-8')  { |f| f.read }

      if string.respond_to?(:encoding) && string.encoding.name != "UTF-8"
        string.encode!('UTF-8')
      end

      begin
        md.instance_eval(string)
      rescue Exception => e
        puts "Invalid `#{File.basename(path)}` file: #{e.message}"
        puts e.backtrace
      end
      
      md.pouet if md.respond_to? :pouet

      if !md.sources.empty? || !md.headers.empty? || !md.assets.empty?
        self.modules << md
        @props[:header_dirs] << md.absolute_path if !md.headers.empty?
      end
    end
    
  end
  
end

