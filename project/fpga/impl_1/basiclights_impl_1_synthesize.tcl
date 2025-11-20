if {[catch {

# define run engine funtion
source [file join {C:/lscc/radiant/2024.2} scripts tcl flow run_engine.tcl]
# define global variables
global para
set para(gui_mode) "1"
set para(prj_dir) "C:/Users/sojayaweera/E155/project/fpga"
if {![file exists {C:/Users/sojayaweera/E155/project/fpga/impl_1}]} {
  file mkdir {C:/Users/sojayaweera/E155/project/fpga/impl_1}
}
cd {C:/Users/sojayaweera/E155/project/fpga/impl_1}
# synthesize IPs
# synthesize VMs
# synthesize top design
file delete -force -- basiclights_impl_1.vm basiclights_impl_1.ldc
::radiant::runengine::run_engine_newmsg synthesis -f "C:/Users/sojayaweera/E155/project/fpga/impl_1/basiclights_impl_1_lattice.synproj" -logfile "basiclights_impl_1_lattice.srp"
::radiant::runengine::run_postsyn [list -a iCE40UP -p iCE40UP5K -t SG48 -sp High-Performance_1.2V -oc Industrial -top -w -o basiclights_impl_1_syn.udb basiclights_impl_1.vm] [list basiclights_impl_1.ldc]

} out]} {
   ::radiant::runengine::runtime_log $out
   exit 1
}
