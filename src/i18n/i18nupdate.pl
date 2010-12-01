#!/usr/bin/perl -w

# i18nupdate.pl
# Utility script that updates source translation files for KLatexFormula
#
# Usage: i18nupdate.pl <filename_XX.ts>
#
# $Id$

$tsfile = $ARGV[0] if $#ARGV >= 0;
if ($#ARGV < 0 || $tsfile eq ""  ||  ! -e $tsfile) {
  print STDERR "Usage: $0 file.ts\n";
  exit 255;
}

# If 1, obsolete texts are kept to be translated.
# Comment out the unrelevant line.
$use_obsolete = 1;
#$use_obsolete = 0;

$sourceslist = "../*.cpp ../klftools/*.cpp ../klfbackend/*.cpp ../plugins/skin/*.cpp ../plugins/systrayicon/*.cpp";
$sourceslist .= " obsolete/*.cpp"  if ($use_obsolete);

$headerslist = "../*.h ../klftools/*.h ../klfbackend/*.h ../plugins/skin/*.h ../plugins/systrayicon/*.h";
$headerslist .= " obsolete/*.h"  if ($use_obsolete);

$formslist = "../*.ui ../klftools/*.ui ../plugins/skin/*.ui ../plugins/systrayicon/*.ui";
$formslist .= " obsolete/*.ui"  if ($use_obsolete);

# Now scan XML files for translatable texts

$xmltrrules =
  {
   "../conf/export_mime_profiles.d/*.xml" => { SCOPE => "exportprofiles" ,
					       TAGS => [ "description" ]
					     } ,
   "../data/latexsymbols.xml"		=> { SCOPE => "latexsymbols" ,
					     TAGS => [ "category-title" ]
					   } ,
   "../plugins/skin/skins/*.xml" => { SCOPE => "pluginskins" ,
				      TAGS => [ "name", "description" ] }
  } ;

$xmltrfile = "temp_xmltr_$tsfile.cxx";

print "Opening file '$xmltrfile' for write\n";
open FXMLTR, '>', $xmltrfile;

$curdate = localtime;
print FXMLTR <<EOXMLCXXHEAD;

/* AUTOMATICALLY GENERATED FILE---DO NOT MODIFY! */

/* This file contains C++ code to feed to ludpate for translatable
   texts in some source XML files. */
/* Automatically generated by $0 on $curdate */

#include <QtCore>

void __klf_i18n_dummy()
{
EOXMLCXXHEAD

sub unxml {
  my $text = shift;
  $text =~ s/&lt;/</g;
  $text =~ s/&gt;/>/g;
  $text =~ s/&amp;/&/g;
  $text =~ s/&quot;/\"/g;
  $text =~ s/&apos;/\'/g;
  $text =~ s/&#(\d+);/chr($1)/eg;
  $text =~ s/&#x([0-9A-Fa-f]+);/chr(hex($1))/eg;
  return $text;
};

# now for each XML source file, see if there are any translatable strings
foreach $f (keys %$xmltrrules) {
  $files = `ls $f`;
  @files = split /\n/, $files;
  print "files: ".join(",", @files)."\n";
  $trscope = $xmltrrules->{$f}{SCOPE};
  @trtaglist = @{$xmltrrules->{$f}{TAGS}};
  $rxtrtaglist = join("|", @trtaglist);
  print "Scope: `$trscope'. Tag list: $rxtrtaglist\n";

 FOREACH_FILE: foreach $file (@files) {
    if ( ! open FXML, '<', $file ) {
      print STDERR "Can't open file `$file': $!\n";
      next FOREACH_FILE;
    }
    $allxml = join("", <FXML>);
    close FXML;
    # now parse XML content
    @trtexts = ();
    $allxml =~ s/<\s*($rxtrtaglist)\s*>([^<]+)<\s*\/\s*\1\s*>/push @trtexts, { text => unxml($2), tag => $1 }/egi;

    # begin an object scope
    $xmlclassname = "xmltr_".$trscope;
    $xmlclassname =~ s/[^A-Za-z0-9_]/_/g;

    print FXMLTR "\n";
    foreach $t (@trtexts) {
      print "Got text to translate: ".$t->{text}.", ".$t->{tag}."\n";

      print FXMLTR "  QT_TRANSLATE_NOOP3(\"$xmlclassname\", \"".$t->{text}."\", \"[[tag: <".$t->{tag}.">]]\");\n";
    }

  }
}

print FXMLTR "\n};\n\n";

close FXMLTR;


# FINALLY RUN LUPDATE

#@allsourceslist = split /\n/, `ls $sourceslist $headerslist $formslist` ;
#
#$lupdatecmd = "lupdate -extensions 'ui,c,cpp,cxx,h' ".
#  join(" ", map "'$_'", @allsourceslist)." '$xmltrfile' -ts '$tsfile'";

#$lupdatecmd = "lupdate -extensions 'ui,c,cpp,cxx,h'  -no-recursive ".
#  " ../ ../klftools/ ../klfbackend/ ../plugins/skin/ ../plugins/systrayicon/ "
#  .($use_obsolete?" obsolete/*.cpp":"")
#  ." -ts '$tsfile'" ;

$tempprofile = "temp_tr_pro_$tsfile.pro";

open FPRO, ">$tempprofile" || die "Can't open file $tempprofile: $!";

print FPRO <<EOFPRO;
#
# AUTOMATICALLY GENERATED FILE---DO NOT MODIFY!
#
# This file contains definitions to be parsed by lupdate for generating
# and updating a translation file.
#
# This project file is meant for use with lupdate only. Don't attempt to parse it
# with qmake.
#
# Automatically generated by $0 on $curdate
#

TEMPLATE = no_template

SOURCES = $sourceslist  $xmltrfile
HEADERS = $headerslist
FORMS   = $formslist

TRANSLATIONS = $tsfile

EOFPRO

close FPRO;

$lupdatecmd = "lupdate $tempprofile";

print "Running: $lupdatecmd\n";
system "$lupdatecmd";

exit 0;
