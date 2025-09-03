if {[catch {

# define run engine funtion
source [file join {C:/lscc/radiant/2024.2} scripts tcl flow run_engine.tcl]
# define global variables
global para
set para(gui_mode) "1"
set para(prj_dir) "C:/Users/sojayaweera/E155/lab2/dual_sevenseg"
if {![file exists {C:/Users/sojayaweera/E155/lab2/dual_sevenseg/top}]} {
  file mkdir {C:/Users/sojayaweera/E155/lab2/dual_sevenseg/top}
}
cd {C:/Users/sojayaweera/E155/lab2/dual_sevenseg/top}
# synthesize IPs
# synthesize VMs
# synthesize top design
file delete -force -- dual_sevenseg_top.vm dual_sevenseg_top.ldc
::radiant::runengine::run_engine_newmsg synthesis -f "C:/Users/sojayaweera/E155/lab2/dual_sevenseg/top/dual_sevenseg_top_lattice.synproj" -logfile "dual_sevenseg_top_lattice.srp"
::radiant::runengine::run_postsyn [list -a iCE40UP -p iCE40UP5K -t SG48 -sp High-Performance_1.2V -oc Industrial -top -w -o dual_sevenseg_top_syn.udb dual_sevenseg_top.vm] [list dual_sevenseg_top.ldc]

} out]} {
   ::radiant::runengine::runtime_log $out
   exit 1
}
