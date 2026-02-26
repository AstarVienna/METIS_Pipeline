#!/usr/bin/perl -pi

BEGIN {

  if (!@ARGV && -t) {
    warn "
  Convert Reflex XML workflows to xml.in format

Usage:

  $0 *.xml.in

  where each xml.in file has been saved from Reflex.

Example:

  cd reflex
  svn commit -m 'Saved from Reflex' visir_image_reduce.xml.in
  $0 visir_image_reduce.xml.in
  svn diff visir_image_reduce.xml.in
  svn commit -m 'Installable Reflex Workflow' visir_image_reduce.xml.in

Limitations:

  A single conversion command is required for each pipeline.

For further support contact llundin\@eso.org

";
     exit;
  }

  # Deduce instrument name from script location
  ($ins) = $0 =~ m#\b(\w+)p/reflex/# or
  ($ins) = $ENV{PWD} =~ m#\b(\w+)p/reflex# or
     die "My location is non-standard: $0";

  %dir = (
  # Input directories
          'ROOT_DATA_DIR', 'ROOT_DATA_PATH_TO_REPLACE/',
          'RAW_DATA_DIR',  '$ROOT_DATA_DIR/reflex_input/'.$ins,
          'CALIB_DATA_DIR', 'CALIB_DATA_PATH_TO_REPLACE/'.$ins.'-@VERSION@/',
  # Working directories
          'BOOKKEEPING_DIR',  '$ROOT_DATA_DIR/reflex_book_keeping/'.$ins,
          'LOGS_DIR',         '$ROOT_DATA_DIR/reflex_logs/'.$ins,
          'TMP_PRODUCTS_DIR', '$ROOT_DATA_DIR/reflex_tmp_products/'.$ins,
# Output directories
          'END_PRODUCTS_DIR', '$ROOT_DATA_DIR/reflex_end_products');

  %scr = (
# Location of files: Python scripts, OCA files
          'Python script',    '@prefix@/share/esopipes/'.$ins.'-@VERSION@/reflex/',
          'OCA File',         '@prefix@/share/esopipes/'.$ins.'-@VERSION@/reflex/');

  %txt = (
# Workflow title(s)
          'text',    '(v. @VERSION@)');

  %par = (
# Configurable parameters that start with a directory
          'prop:', '@prefix@/share/esopipes/'.$ins.'-@VERSION@/config/');

  $mdir = join('|', keys %dir);
  $ndir = scalar keys %dir;

  $mscr = join('|', keys %scr);
  $mtxt = join('|', keys %txt);
  $mpar = join('|', keys %par);
};

END {

  for $argv (keys %argv) {

    for $scr (keys %scr) {
      $done{$argv}{$scr} ||= 0;
      warn("Updated $done{$argv}{$scr} '$scr' location(s) for: $argv\n");
    }

    for $txt (keys %txt) {
      $done{$argv}{$txt} ||= 0;
      warn("Updated $done{$argv}{$txt} '$txt' version(s) for: $argv\n");
    }

    for $par (keys %par) {
      $done{$argv}{$par} ||= 0;
      warn("Updated $done{$argv}{$par} '$par' location(s) for: $argv\n");
    }

    # Verify that all directory replacements have been done
    $die = 0;

    $idir = 0;
    for $dir (keys %dir) {
      $done{$argv}{$dir} == 1 or $die++,
        warn("$argv does not define $dir"),
        next;
      $idir++;
    }
    $idir == $ndir or $die++,
        warn("$argv defines $idir input directories, not these $ndir: $mdir");

    $die and die "$argv had $die error(s)";
    warn("Updated $ndir (mandatory) directories for: $argv\n");
  }
};

# Keep track of processed files, for format verification and diagnostics
$argv{$ARGV}++;

# Update directories
s#(<property name="($mdir)" class="ptolemy.data.expr.FileParameter" value=")[^\"]*(">)#$1$dir{$2}$3# && $done{$ARGV}{$2}++ && die("$ARGV defines $2 more than once: $_\n");

# Update zero, one or more instances of locations of OCA-files and (Python) scripts
# - also, verify that the file exists
s#(<property name="($mscr)" class="ptolemy.data.expr.FileParameter" value=")[^\"<>]*/([^\"<>\/]+)#$1$scr{$2}$3# and $done{$ARGV}{$2}++,
  -e "$3" || die("$ARGV includes a file '$3' of type '$2' that does not exist: $_\n");

# Update zero, one or more instances of texts with a version number
s#(<property name="($mtxt)" class="ptolemy.kernel.util.StringAttribute" value="[^\"]*)\(v\.\s*[^\s\)\"]+\)#$1$txt{$2}# && $done{$ARGV}{$2}++;

# Update zero, one or more instances of parameters starting with a directory
# FIXME: Does this cover all relevant cases ?
s#(<property name="($mpar)\w+" class="ptolemy.kernel.util.ConfigurableAttribute"><configure>[^<>]*Default value:)/[^<>]*/([^<>]*</configure>)#$1$par{$2}$3# && $done{$ARGV}{$2}++;
