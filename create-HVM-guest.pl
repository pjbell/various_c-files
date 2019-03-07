#!/usr/bin/perl
#
# $Header: /home/rcitton/CVS/create-HVM-guest/create-HVM-guest.pl,v 1.26 2016/08/24 09:20:01 rcitton Exp $
#
# create-HVM-guest.pl
#
# Copyright (c) 2014, Oracle and/or its affiliates. All rights reserved.
#
#    NAME
#      create-HVM-guest - Create an HVM guest
#
#    DESCRIPTION
#      Create an HVM guest from an ISO OS image
#
#    NOTES & LIMITATIONS
#       1. Due to bug 17804147 & bug 18087068, only shared repo are supported
#       2. I'm using Sparse file, ACFS is supporting them since 12.1.2.0.0
#       3. The ISO image must be available on dom0
#
#
#    MODIFIED   (MM/DD/YY)
#    rcitton     08/24/16 - Cleanup in case of failure
#    rcitton     08/24/16 - Node number
#    rcitton     04/27/16 - pvhvm option
#    rcitton     08/07/15 - fixed size using G
#    rcitton     08/07/15 - start -d option
#    rcitton     08/07/15 - VM in snapshot
#    rcitton     08/07/15 - VNC port feature
#    rcitton     01/13/14 - Creation
#

### ------------------------------------------------------------------------
### DISCLAIMER:
###    It is NOT supported by Oracle World Wide Technical Support.
###    The script has been tested and appears to work as intended.
###    You should always run new scripts on a test instance initially.
### ------------------------------------------------------------------------

=head1 NAME

 create-HVM-guest.pl - Script to create an HVM guest from an ISO OS image

=head1 SYNOPSIS

 create-HVM-guest.pl -name <HVM guest host name>
                     -memory <memory Gb> -vcpu <number of vcpu> -vdisksize <virtual system disk size Gb>
                     -iso <ISO OS image files location on Dom0>
                     -repo <repository name>
                     [-vnc <port>]
                     [-pvhvm]

 create-HVM-guest OPTIONS
    -name       HVM guest host name
    -memory     HVM guest memory setting Gb
    -vcpu       HVM guest vcpu setting
    -vdisksize  HVM guest system disk size Gb
    -iso        ISO OS image files location on Dom0
    -repo       VM repository name
    -vnc        VNCVierwer Port (default first unused port above 5900)
    -pvhvm      If used, network type will be 'netfront' for PVHVM guest (default 'ioemu' suitable for Ms Windows guest deploy)

 Examples:
  create-HVM-guest.pl -name MsWin7 -memory 4 -vcpu 2 -vdisksize 200 -iso /OVS/win7_x64.iso -repo sharedrepo
  create-HVM-guest.pl -name MsWin7 -memory 4 -vcpu 2 -vdisksize 200 -iso /OVS/win7_x64.iso -repo sharedrepo -vnc 10
  create-HVM-guest.pl -name OPSuse -memory 4 -vcpu 2 -vdisksize 200 -iso /OVS/OpenSuse.iso -repo sharedrepo -vnc 20 -pvhvm

=head1 DESCRIPTION

  The purpose of this prototype is to automate the Fully-Virtualized Guest (Hardware Virtual Machine)
  creation process, from an os iso image (cd/dvd) on Oracle Database Appliance Virtualized Platform

=head1 LIMITATIONS

  1. Only shared repo are supported
  2. I'm using Sparse file, ACFS supports them starting with OAK 12.1.2.0.0
  3. The ISO image must be available on dom0

=head1 BUGS

 No known bugs.

=head1 AUTHOR

 Written by Ruggero Citton(Ruggero.Citton@oracle.com)

=head1 COPYRIGHT

 Copyright (c) 2013, 2014, Oracle and/or its affiliates. All rights reserved.

=head1 DISCLAIMER

 This script is provided for educational purposes only.
 It is NOT supported by Oracle World Wide Technical Support.
 The script has been tested and appears to work as intended.
 You should always run new scripts on a test instance initially.

=head1 VERSION

 20150807.1.01 $Revision: 1.26 $

=cut


use strict;
use warnings;
use English;
use File::Spec::Functions;
use Getopt::Long;
use Pod::Usage;
my $OAK_REPOS_HOME;


BEGIN
{
   $OAK_REPOS_HOME = $ENV{'OAK_REPOS_HOME'};
   if (!$OAK_REPOS_HOME)
   {
      $OAK_REPOS_HOME = "/opt/oracle/oak";
   }
   my $oak_lib = catfile($OAK_REPOS_HOME,"lib","oaklib");
   my $oak_utils_lib = catfile($OAK_REPOS_HOME,"lib","oakutilslib");
   my $onecmd_lib = catfile($OAK_REPOS_HOME,"onecmd");
   my $onecmd_osd_lib = catfile($OAK_REPOS_HOME,"onecmd","Linux");
   push @INC, $oak_lib;
   push @INC, $oak_utils_lib;
   push @INC, $onecmd_lib;
   push @INC, $onecmd_osd_lib;

}

use oakutils;
use oakLoggingAndTracing;

my $name;
my $memory;
my $vcpu;
my $vdisksize;
my $iso;
my $repo;
my $vnc;
my $pvhvm;
my $help = 0;

my $OH;
my $host;
my $tport;
my $socket;
my $pid;


# ----------------------------------------------------------------------
# Interrupts definition
# ----------------------------------------------------------------------
$SIG{'INT'} = 'INT_handler';
$SIG{'QUIT'} = 'INT_handler';
$SIG{'TERM'} = 'INT_handler';


# ----------------------------------------------------------------------
# Command line options
# ----------------------------------------------------------------------
if ( @ARGV > 0 ) {
    GetOptions (
    'name=s'       => \$name,
    'memory=i'     => \$memory,
    'vcpu=i'       => \$vcpu,
    'vdisksize=i'  => \$vdisksize,
    'iso=s'        => \$iso,
    'repo=s'       => \$repo,
    'vnc=i'        => \$vnc,
    'pvhvm'        => \$pvhvm,
    'help'         => \$help
    )
    or pod2usage(-verbose => 0, -message => "$0: argument required\n");
}
if ( $help ) {pod2usage(-verbose => 1);}

# ----------------------------------------------------------------------
# Sub common functions
# -----------------------------------------------------------------------
sub my_trim_data
{
  my ($data) = @_;
  if (defined($data)) {
    $data =~ s/^\s+//;
    $data =~ s/\s+$//;
    return $data;
  }
}

# --------------------------------------------------------------------------------------------------------
# Sub create-HVM-guest functions
# -------------------------------------------------------------------------------------------------------------------------------

sub validateOptions {
  if (!defined($name) or !defined($memory) or !defined($vcpu) or !defined($vdisksize) or !defined($iso) or !defined($repo))
  {
    trace("E","Missing command option\n");
    pod2usage(2);
  }
}

sub checkUser {
  if ( 'root' ne getpwuid( $< ))
  {
    trace("E","This should be run as root user. Exiting...");
    exit(1);
  }
}

sub checkVM {
  my $vm = getVmINfo($name,'OAKERR');
  if ($vm eq ''){
    trace("E","The VM $name is present. Exiting...");
    exit(1);
  }
}

sub checkVMTemplate {
  my $vmtemplate = getVmTemplateInfo($name,'OAKERR');
  if ($vmtemplate eq ''){
    trace("E","The VMTemplate $name is present. Exiting...");
    exit(1);
  }
}

sub setupEnv {
  $OH='/opt/oracle/oak/bin';
  system("mkdir -p /tmp/vm_tmp/$name");
}

sub getVmINfo
{
  my ($vm_name, $sstring) = @_;
  my $cmd = "$OH/oakcli show vm $vm_name";

  open (my $RES, "$cmd |");
  my @search = grep /$sstring/, <$RES>;
  close $RES;

  my @value;
  if (defined $search[0]){
    @value = split(":", $search[0]);
  }

  return (my_trim_data($value[1]));
}

sub getVmTemplateInfo
{
  my ($vm_name, $sstring) = @_;
  my $cmd = "$OH/oakcli show vmtemplate $vm_name";

  open (my $RES, "$cmd |");
  my @search = grep /$sstring/, <$RES>;
  close $RES;

  my @value;
  if (defined $search[0]){
    @value = split(":", $search[0]);
  }

  return (my_trim_data($value[1]));
}

sub getRepoINfo
{
  my ($repo_name, $sstring) = @_;
  my $cmd = "$OH/oakcli show repo $repo -node 0";

  open (my $RES, "$cmd |");
  my @search = grep /$sstring/, <$RES>;
  close $RES;

  my @value;
  if (defined $search[1]){
    @value = split(":", $search[1]);
  }
  return ($value[1]);
}

sub checkRepo{
  if ( ($repo ne 'odarepo1') and ($repo ne 'odarepo2') ) {
    my $sysVer = getSystemVersion();
    my $check = compareVersion('2.8.0.0.0', $sysVer);
    if ( $check ne '1') {
      trace("E","Shared repository option is valid on ODA version 2.8.0.0.0 and above, your version is $sysVer . Exiting...");
      exit(1);
    }

    my $rc = system_cmd ("$OH/oakcli show repo $repo -node 0|grep 'Could not find the Resource'");
    if ($rc eq '0') {
      trace("E","The sharedrepo $repo does not exists, please create this first and rerun the create script.");
      errorExit();
    }

    my $repoinfo = getRepoINfo($repo, "State");
    if (defined($repoinfo)) {
      $repoinfo = my_trim_data($repoinfo);
    }
    else {
      trace("E","The sharedrepo $repo does not exists, please create this first and rerun the create script, exiting...");
      errorExit();
    }
    if ($repoinfo ne 'Online') {
      trace("E","The sharedrepo $repo exists, but it's not online.");
      errorExit();
    }
  }
  else {
    trace("E","create-HVM-guest is supported with shared repository only. Exiting...");
    exit(1);
  }
}

sub cleanUp {
  my @output = system_cmd_capture("$OH/oakcli stop vm $name -force");
  @output = system_cmd_capture("$OH/oakcli delete vm $name");
  @output = system_cmd_capture("$OH/oakcli delete vmtemplate $name");
  my $deleteregdir = "/tmp/vm_tmp/$name";
  system ("rm -fr $deleteregdir");
}

sub cleanStage {
  my $deleteregdir = "/tmp/vm_tmp/$name";
  system ("rm -fr $deleteregdir");
}

sub errorExit {
  if (defined($pid)) { system_cmd("kill -9 $pid"); }
  cleanUp;
  exit(1);
}

sub INT_handler {
  trace("E","Exiting due to interrupt");
  errorExit;
}

sub getHostInfo {
    trace("I","Getting host info...");
    my @hostname = system_cmd_capture("hostname -f");
    if ($hostname[0] != 0) {
        trace("E","Error getting the hostname");
        exit(1);
    }
    $host = $hostname[1];
    $host = my_trim_data($host);
}

sub createVMConf {
  my $vm_conf="/tmp/vm_tmp/$name/vm.cfg";
  $memory = $memory * 1024;
  open(VMC,">$vm_conf") or die("ERROR: Cannot open $vm_conf at source host $!");
  print VMC  "##HVM Guest vm.cfg by create-HVM-guest one-button\n";
  print VMC  "name = '$name'\n";
  print VMC  "kernel = '/usr/lib/xen/boot/hvmloader'\n";
  print VMC  "device_model = '/usr/lib/xen/bin/qemu-dm'\n";
  print VMC  "builder = 'hvm'\n";
  print VMC  "memory = '$memory'\n";
  print VMC  "vcpus = '$vcpu'\n";
  print VMC  "boot = 'dc'\n";
  print VMC  "acpi = 1\n";
  print VMC  "apic = 1\n";
  print VMC  "on_reboot = 'destroy'\n";
  print VMC  "on_crash = 'destroy'\n";
  print VMC  "on_poweroff = 'destroy'\n";
  print VMC  "usbdevice = 'tablet'\n";
  print VMC  "localtime = 1\n";
  print VMC  "disk = [ 'file:/tmp/vm_tmp/$name/System.img,xvda,w' ]\n";
  if ( defined($pvhvm) ){
    print VMC  "vif = [ 'type=netfront, bridge=net1' ]\n";
  }
  else {
    print VMC  "vif = [ 'type=ioemu, bridge=net1' ]\n";
  }
  if ( defined($vnc) ){
    print VMC  "vfb = [ 'type=vnc,vnclisten=0.0.0.0,vncdisplay=$vnc' ]\n";
  }
  else {
    print VMC  "vfb = [ 'type=vnc,vnclisten=0.0.0.0,vncunused=1' ]\n";
  }
  print VMC  "##HVM Guest vm.cfg by create-HVM-guest one-button\n";
  close(VMC);
}

sub createVDisk {
  my $rc = system_cmd("dd if=/dev/zero of=/tmp/vm_tmp/$name/System.img oflag=direct bs=1 count=0 seek=${vdisksize}G");
  if ( $rc eq 0 ) {
    trace("I","VM guest virtual disk created!");
  }
  else {
    trace("E","VM guest virtual disk creation failed!");
    errorExit;
  }

}

sub createVMTemplate {
  trace("I","Creating VM guest temporary template in progress, it may need some time");
  my $rc = system_cmd("cd /tmp/vm_tmp/$name; tar cvfzS $name.tgz *.img vm.cfg");
  if ( $rc eq 0 ) {
    trace("I","VM guest temporary template created!");
  }
  else {
    trace("E","VM guest temporary template  creation failed!");
    errorExit;
  }
}

sub getFreePort {
  trace("I","Getting free port...");
  for ($tport=36000; $tport<=37000; $tport++) {
      my $success = eval {
          $socket = IO::Socket::INET->new(
          LocalAddr        => $host,
          LocalPort        => $tport,
          Proto            => 'tcp'
          )
      };
      if ($success) {
          $socket->close;
          last;
      };
  };
}

sub startHTTP {
  trace("I","Starting Simple HTTP server");
  my $rc = system("cd /tmp/vm_tmp/$name; /usr/bin/python -m SimpleHTTPServer $tport & ");

  my @output = system_cmd_capture("pgrep -fl 'SimpleHTTPServer $tport'");
  my @verarray = split(" ",$output[1]);

  my $verarray;
  $pid = $verarray[0];

  if ( $rc eq '0' ) {
    trace("I","Simple HTTP server started!");
  }
  else {
    trace("E","Simple HTTP server failed!");
    errorExit;
  }
  sleep 2;
}

sub stopHTTP {
  my $rc = system_cmd("kill -9 $pid");
  if ( $rc eq '0' ) {
    trace("I","Simple HTTP server stopped!");
  }
  else {
    trace("E","Stopping Simple HTTP server failed!");
    errorExit;
  }
}

sub impTemplate {
  trace("I","Importing $name VM Templates...");
  my @output;
  if ( ($repo ne 'odarepo1') and ($repo ne 'odarepo2') ) {
    @output = system_cmd_capture("$OH/oakcli import vmtemplate $name -files 'http://$host:$tport/$name.tgz' -repo $repo -node 0");
  }
  else {
    @output = system_cmd_capture("$OH/oakcli import vmtemplate $name -files 'http://$host:$tport/$name.tgz' -repo $repo");
  }
  my @verarray = split(" ",$output[2]);
  if ($verarray[0] ne 'Imported') {
      if ($output[2] eq 'OAKERR:8004 Template $name already exists') {
          trace("W","Tempalte $name already imported, skiping...");
      }
      else  {
          trace("E","Import $name on $repo failed");
          trace("E","Error: $output[2]");
          errorExit();
      }
  }
  trace("I","VM Templates $name imported on $repo...");
}

sub cloneVM {
  trace("I","Cloning $name...");
  my @output;
  if ( ($repo ne 'odarepo1') and ($repo ne 'odarepo2') ) {
    @output = system_cmd_capture("$OH/oakcli clone vm $name -vmtemplate $name -repo $repo -node 0");
  }
  else {
    @output = system_cmd_capture("$OH/oakcli clone vm $name -vmtemplate $name -repo $repo");
  }
  if ($output[2] ne "Cloned VM : $name") {
      if ($output[2] eq 'OAKERR:8003 VM $name already exists') {
          trace("W","VM $name already cloned exiting...");
          errorExit();
      }
      else  {
          trace("E","Clone $name failed");
          trace("E","Error: $output[2]");
          errorExit();
      }
  }
}

# RC bug 17804147 - need to edit manualy vm.cfg
#sub editVM {
#  trace("I","Editing $name.....");
#  my $disk = "['file:/tmp/vm_tmp/$name/System.img,xvda,w','file:$iso,xvdc:cdrom,r']";
#  my @output = system_cmd_capture("$OH/oakcli configure vm $name -disk \"$disk\"");
#  if ($output[2] ne "Configured VM : $name. The settings will take effect upon the next restart of this VM.") {
#      trace("E","Setup disk for $name failed");
#      trace("E","Error: $output[2]");
#      errorExit();
#  }
#}

# RC bug 17804147 - need to edit manualy vm.cfg workaround on sharedrepo
sub editVM {
  trace("I","Editing $name configuration file...");
  my $vmcfg = "/u01/app/sharedrepo/$repo/.ACFS/snaps/$name/VirtualMachines/$name/vm.cfg";
  my $olddisk = "disk =";
  open( FILE, "<$vmcfg" ) or die ("Could not open $vmcfg, $!");
  my @LINES = <FILE>;
  close( FILE );

  open( FILE, ">$vmcfg" ) or die ("Could not open $vmcfg, $!");
  foreach my $LINE ( @LINES ) {
    print FILE $LINE unless ( $LINE =~ m/$olddisk/ );
  }
  print FILE  "disk = ['file:/OVS/Repositories/$repo/.ACFS/snaps/$name/VirtualMachines/$name/System.img,xvda,w','file:$iso,xvdc:cdrom,r']\n";
  close( FILE );
}

sub setPrefNode {
  my $node_number = getNodeNumber();
  trace("I","Setting up the prefered node to $node_number");
  my @output = system_cmd_capture("$OH/oakcli configure vm $name -prefnode $node_number");
  if (! $output[2] =~ m/take effect/){
    trace("E","Setup prefered node for $name failed");
    trace("E","Error: $output[2]");
    errorExit();
  }
}

sub startupVM {
  my @output = system_cmd_capture("$OH/oakcli start vm $name -d");
  if ($output[2] =~ m/Started VM/){
    trace("I","==================================");
    trace("S","VM Guest $name is now running!");
    trace("I","==================================");
    trace("I","You should now complete manually the OS Installation");
  }
  else {
    trace("E","Starting the $name failed");
    trace("E","Error: $output[2]\n $output[3]\n $output[4]");
    errorExit();
  }
}


sub connnectVNC {
  my $node_number = getNodeNumber();
  trace("I","Trying to open a graphical VM console for $name");
  my @output = system_cmd_capture("$OH/oakcli show vmconsole $name &");

  my $myport;
  if (grep(/OAKERR/,@output)){
    if (defined($vnc)){
      $myport = $vnc;
    }
    else {
      $myport = getVmINfo($name,'vncport');
      if ($myport eq ''){
        $myport = '<first unesed port above 5900>';
      }
    }
    trace("W","You are not running on an X graphical environment");
    trace("W","Connect your VM guest $name using VNC dom0-node$node_number-hostname:$myport");
  }
}


# RC bug 18087068 - need to edit manualy vm.cfg
#sub changeBootOrder {
#  trace("I","Changing boot order on $name configuration file.....");
#  my $boot = "'cd'";
#  my @output = system_cmd_capture("$OH/oakcli configure vm $name -bootoption \"$boot\"");
#  if ($output[2] ne "Configured VM : $name. The settings will take effect upon the next restart of this VM.") {
#      trace("E","Boot order for $name failed");
#      trace("E","Error: $output[2]");
#      errorExit();
#  }
#}
sub changeBootOrder {
  trace("I","Changing boot order on $name configuration file...");
  my $vmcfg = "/u01/app/sharedrepo/$repo/.ACFS/snaps/$name/VirtualMachines/$name/vm.cfg";
  my $entry1 = "boot =";
  my $entry2 = "disk =";
  open( FILE, "<$vmcfg" ) or die ("Could not open $vmcfg, $!");
  my @LINES = <FILE>;
  close( FILE );
  open( FILE, ">$vmcfg" );
  foreach my $LINE ( @LINES ) {
    print FILE $LINE unless ( ($LINE =~ m/$entry1/) or ($LINE =~ m/$entry2/) );
  }
  print FILE  "disk = ['file:/OVS/Repositories/$repo/.ACFS/snaps/$name/VirtualMachines/$name/System.img,xvda,w']\n";
  close( FILE );
}
# ========================================================================================================
# ========================================================================================================


validateOptions;
trace("I","Validating environment...");
setupEnv;
checkUser;
checkRepo;
checkVM;
checkVMTemplate;

if ( defined($pvhvm) ){
  trace("I","PVHVM guest deploy in progress...");
  trace("W","Network type will be 'netfront'");
  trace("W","Note: this is not a recommended setting for initial Ms Windows guest deploy");
}
else {
  trace("I","HVM guest deploy in progress...");
  trace("W","Network type will be 'ioemu'");
  trace("W","Note: this is not a recommended setting for PVHVM guest deploy");
}
createVMConf;
createVDisk;
createVMTemplate;
getHostInfo;
getFreePort;
startHTTP;
impTemplate;
stopHTTP;
cloneVM;
editVM;
setPrefNode;
startupVM;
connnectVNC;
changeBootOrder;
cleanStage;


# ========================================================================================================
# ========================================================================================================
[root@ovcamn06r1 ISO]#
