#!/usr/bin/env ruby

root_dir = File.absolute_path '.'
$LOAD_PATH << "#{root_dir}/scripts"

require 'project.rb'
require 'qt_project.rb'

proj = QtProject.new root_dir
proj.parse_projfile "#{root_dir}/scripts/reflow-qt.projfile"
proj.build_in_place! "Reflow"
