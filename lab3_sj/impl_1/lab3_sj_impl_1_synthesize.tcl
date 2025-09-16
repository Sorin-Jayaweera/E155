if {[catch {

# define run engine funtion
source [file join {C:/lscc/radiant/2024.2} scripts tcl flow run_engine.tcl]
# define global variables
global para
set para(gui_mode) "1"
set para(prj_dir) "C:/Users/sojayaweera/E155/lab3_sj"
if {![file exists {C:/Users/sojayaweera/E155/lab3_sj/impl_1}]} {
  file mkdir {C:/Users/sojayaweera/E155/lab3_sj/impl_1}
}
cd {C:/Users/sojayaweera/E155/lab3_sj/impl_1}
# synthesize IPs
# synthesize VMs
::radiant::runengine::run_postsyn [list -a iCE40UP -p iCE40UP5K -t SG48 -sp High-Performance_1.2V -oc Industrial -w -o testbench_counter_7b293bf5d66558a2819c39f487cde2ac.udb C:/Users/sojayaweera/E155/lab3_sj/source/impl_1/testbench_counter.vm] [list {}]
# synthesize top design
file delete -force -- lab3_sj_impl_1.vm lab3_sj_impl_1.ldc
::radiant::runengine::run_engine_newmsg synthesis -f "C:/Users/sojayaweera/E155/lab3_sj/impl_1/lab3_sj_impl_1_lattice.synproj" -logfile "lab3_sj_impl_1_lattice.srp"
::radiant::runengine::run_postsyn [list -a iCE40UP -p iCE40UP5K -t SG48 -sp High-Performance_1.2V -oc Industrial -top -iplist iplist.txt -w -o lab3_sj_impl_1_syn.udb lab3_sj_impl_1.vm] [list lab3_sj_impl_1.ldc]

} out]} {
   ::radiant::runengine::runtime_log $out
   exit 1
}
