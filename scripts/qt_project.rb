include ObjectSpace

class QtProject < Project
  
  def path_relative_to_outdir(file)
    p0 = Pathname.new(File.dirname(file))
    p1 = Pathname.new(@project_dir)
    filepath = File.join p0.relative_path_from(p1), File.basename(file)
    filepath
  end
  
  def write_pro_to_file!(f)
    f.puts "QT += core gui widgets xml"
    f.puts "CONFIG += c++11"
    f.puts "TARGET = #{self.prop(:product_name)}"
    f.puts "TEMPLATE = app"
    f.puts "RESOURCES += #{@project_name}.qrc"
    
    # Defines
    @props[:defines][:all].each do |d|
      f.puts "DEFINES += #{d}"
    end
    
    # Debug/Release spec
    f.puts "CONFIG(debug, debug|release) {"
      @props[:defines][:debug].each do |d|
        f.puts "DEFINES += #{d}"
      end
    f.puts "}"
    f.puts "else {"
      @props[:defines][:release].each do |d|
        f.puts "DEFINES += #{d}"
      end
    f.puts "}"
    
    # Source, Header and Form Files
    @modules.each do |md|
      md.sources.each do |src|
        filepath = path_relative_to_outdir src
        f.puts "SOURCES += \"#{filepath}\""
      end
      
      md.headers.each do |header|
        filepath = path_relative_to_outdir header
        f.puts "HEADERS += \"#{filepath}\""
      end
      
      md.forms.each do |form|
        filepath = path_relative_to_outdir form
        f.puts "FORMS += \"#{filepath}\""
      end
    end
    
    # Includes
    @props[:header_dirs].each do |inc|
      p0 = Pathname.new(inc)
      p1 = Pathname.new(@project_dir)
      path = p0.relative_path_from p1
      f.puts "INCLUDEPATH += \"#{path}\""
    end
    
    # Windows spec
    f.puts "win32 {"
    f.puts "    INCLUDEPATH += \"C:\\boost_1_52_0\""
    f.puts "    INCLUDEPATH += \"C:\\Program Files (x86)\\Jack\\includes\""
    f.puts "    DEFINES += BOOST_MEM_FN_ENABLE_STDCALL"
    f.puts "    DEFINES += __WINDOWS_DS__"                  # RtAudio will use DirectSound
    f.puts "    QMAKE_LIBDIR += \"C:\\Program Files (x86)\\Jack\\lib\""
    f.puts "    LIBS += dsound.lib ole32.lib libjack.lib"
    f.puts "    RC_FILE = Reflow.rc"
    f.puts "}"
    
    # Mac spec
    f.puts "macx {"
    f.puts "  INCLUDEPATH += /opt/Boost"
    f.puts "  INCLUDEPATH += /usr/local/include"
    f.puts "  DEFINES += __MACOSX_CORE__"
    f.puts "  DEFINES += MACOSX"
    f.puts "  LIBS += -lpthread -L/usr/local/lib -ljack -framework CoreAudio -framework CoreFoundation"
    f.puts "}"

    # Linux spec
    f.puts "linux-clang {"
    f.puts "    DEFINES += LINUX __LINUX_PULSE__"
    f.puts "    QMAKE_CXXFLAGS += -std=c++11"
    f.puts "    LIBS += -lpulse -lpulse-simple -ljack -lpthread -lrt"
    f.puts "}"
    
  end
  
  def write_qrc_to_file!(f)
    f.puts "<RCC>"
    f.puts "  <qresource prefix='/'>"
    @modules.each do |md|
      md.assets.each do |asset|
        filepath = path_relative_to_outdir(asset)
        f.puts "    <file alias=\"#{File.basename(asset)}\">#{filepath}</file>"
      end
    end
    f.puts "  </qresource>"
    f.puts "</RCC>"
  end
  
  def build!(project_name)
    FileUtils.mkpath(self.out_dir) if not Dir.exists?(self.out_dir)
    
    @project_dir = self.out_dir + "/#{project_name}"
    if not Dir.exists?(@project_dir) then
      Dir.mkdir(@project_dir) 
    end
    
    @project_name = project_name
    
    # Write .qrc
    @qrc_path = File.join(@project_dir, "#{project_name}.qrc")
    File.open(@qrc_path, "w") do |file|
      write_qrc_to_file! file
    end
    
    # Write .pro
    @pro_path = File.join(@project_dir, "#{project_name}.pro")
    File.open(@pro_path, "w") do |file|
      write_pro_to_file! file
    end
  end
  
  def build_in_place!(project_name)
    
    self.out_dir = self.root_dir
    @project_dir = self.out_dir
    
    @project_name = project_name
    
    # Write .qrc
    @qrc_path = File.join(@project_dir, "#{project_name}.qrc")
    File.open(@qrc_path, "w") do |file|
      write_qrc_to_file! file
    end
    
    # Write .pro
    @pro_path = File.join(@project_dir, "#{project_name}.pro")
    File.open(@pro_path, "w") do |file|
      write_pro_to_file! file
    end
  end
  
  def initialize(rd)
    super(rd)
  end
end
