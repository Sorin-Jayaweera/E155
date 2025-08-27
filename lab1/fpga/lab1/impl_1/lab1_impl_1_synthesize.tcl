if {[catch {

# define run engine funtion
source [file join {C:/lscc/radiant/2025.1} scripts tcl flow run_engine.tcl]
# define global variables
global para
set para(gui_mode) "1"
set para(prj_dir) "C:/ThatFamily Dropbox/Sorin Jayaweera/allSaves/mudd/junior/MicroPs/lab1/fpga/lab1"
if {![file exists {C:/ThatFamily Dropbox/Sorin Jayaweera/allSaves/mudd/junior/MicroPs/lab1/fpga/lab1/impl_1}]} {
  file mkdir {C:/ThatFamily Dropbox/Sorin Jayaweera/allSaves/mudd/junior/MicroPs/lab1/fpga/lab1/impl_1}
}
cd {C:/ThatFamily Dropbox/Sorin Jayaweera/allSaves/mudd/junior/MicroPs/lab1/fpga/lab1/impl_1}
# synthesize IPs
# synthesize VMs
# synthesize top design
file delete -force -- lab1_impl_1.vm lab1_impl_1.ldc
::radiant::runengine::run_engine_newmsg synthesis -f "C:/ThatFamily Dropbox/Sorin Jayaweera/allSaves/mudd/junior/MicroPs/lab1/fpga/lab1/impl_1/lab1_impl_1_lattice.synproj" -logfile "lab1_impl_1_lattice.srp"
::radiant::runengine::run_postsyn [list -a iCE40UP -p iCE40UP5K -t SG48 -sp High-Performance_1.2V -oc Industrial -top -w -o lab1_impl_1_syn.udb lab1_impl_1.vm] [list lab1_impl_1.ldc]

} out]} {
   ::radiant::runengine::runtime_log $out
   exit 1
}
