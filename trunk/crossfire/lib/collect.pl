#!/usr/local/bin/perl

require "util.pl";

$root = $ARGV[0];
$archetypes = "archetypes";
$bmaps = "bmaps";
$faces = "faces";
$animations = "animations";
$paths = $bmaps."."."paths";
$faceExt = "\\.[a-zA-Z0-9][A-Z0-9][A-Z0-9]";

### main
&info("looking ...");
&traverse($root);

&info("writing ...$archetypes");
open(ARCH,">".$archetypes) || &die("cannot open ".$archetypes);
&archsOut;
close(ARCH);

&info("$bmaps");
open(BMAPS,">".$bmaps) || &die("cannot open ".$bmaps);
&bmapsOut;
close(BMAPS);
open(BMAPS,">".$paths) || &die("cannot open ".$paths);
&pathsOut;
close(BMAPS);

&info("$faces");
open(FACES,">".$faces) || &die("cannot open ".$faces);
&facesOut;
close(FACES);

&info("$animations");
open(ANIM,">".$animations) || &die("cannot open ".$animations);
&animOut;
close(ANIM);

&stats;
exit 0;

sub traverse {
    local($dir) = shift;
    local($file,$name);
    local( $tfile);

    opendir(THISDIR, $dir) || die "couldn't open $dir";
    local(@allfiles) = readdir(THISDIR);
    closedir(THISDIR);

    foreach $tfile (@allfiles) {
	next if $tfile =~ /^\./;
	$file = $dir."/".$tfile;
	$name = &basename($file,""); # DIR

        if( -d $file && $name ne "dev" && $name ne "trashbin" && $name ne "CVS" ) {
	    &traverse($file);
	} elsif ( -d $file && ( $name eq "dev" || $name eq "trashbin" ) ) {
# Empty directive to prevent warnings below
	} elsif( $file =~ /.*\.arc$/) {	# ARCHETYPE
	    $archsNum++;
	    push(@archs,$file);
	} elsif( $file =~ /.*$faceExt$/) { # FACE
	    $facesNum++;
	    &warn("duplicate face ".$name." in ".$dir." and $faces{$name}")
		if $faces{$name};
	    $faces{$name} = $file;
	    # &warn($file." has no archetype") 
	    #	if ! -r $dir."/".&basename($file,$faceExt).".arc";
	} elsif( $file =~ /.*$faceExt.xpm$/ || $file =~ /.$faceExt.png$/ ) { # FACE
# Empty directive to prevent warnings below
	} elsif ( $file =~ /.*\.face$/) {	# Face information file
	    $facesFileNum++;
	    push(@face_files, $file);
	} 
	# ignore a couple of the more common 'junk' files that are not
	# really junk.
	elsif (($name ne "README") && ($name ne "CVS")) {
	    $trashNum++;
	    print "Warning: $file might be a junk file\n";
	}
    }
}

sub storeFaceInfo {
    local($lface,@values) = @_;

    if ($values[0] ne "") {
#	blank.111 is a special case -
#	since no foreground pixels will actually be drawn, foreground colors is
#	not relevant.  Several monsters use blank.111 as part of their
#	animation to make them appear invisible, but have some other
#	foreground color set.
#	Same applies to empty also.
	if ($fg{$lface} && $fg{$lface} ne $values[0] && $lface ne "blank.111"
		&& $lface ne "empty.111") {
	    &warn($arch." duplicate fg color ".$fg{$lface}."/".$values[0]." face ".$lface);
	} else {
		$fg{$lface} = $values[0];
	}
    }
    if ($values[1] ne "") {
	if ($bg{$lface} && $bg{$lface} ne $values[1]) {
	    &warn($arch." duplicate bg color ".$bg{$lface}."/".$values[1]." face ".$lface);
	} else {
		$bg{$lface} = $values[1];
	}
    }
    if ($values[2] ne "" && $lface ne "blank.111" && $lface ne "empty.111") {
#      blank.111 is a special case - see above explanation
#      Its visibility is always 0.
	if ($visibility{$lface} && $visibility{$lface} ne $values[2]) {
	    &warn($arch." duplicate visibilty ".$visibility{$lface}."/".$values[2]." face ".$lface);
	} else {
		$visibility{$lface} = $values[2];
	}
    }
    if ($values[3] ne "" && lface ne "blank.111" && $lface ne "empty.111") {
	if ($magicmap{$lface} && $magicmap{$lface} ne $values[3]) {
	    &warn($arch." duplicate magicmap color ".$magicmap{$lface}."/".$values[3]." face ".$lface);
	} else {
		$magicmap{$lface} = $values[3];
	}
    }
    if ($values[4] ne "") {
	if ($floor{$lface} && $floor{$lface} ne $values[4]) {
	    &warn($arch." duplicate floor information ".$floor{$lface}."/".$values[4]." face ".$lface);
	} else {
		$floor{$lface} = $values[4];
	}
    }
}
	

sub archsOut {
    foreach $arch (@archs) {
	open(ARC,$arch) || &die("cannot open ".$arch);
line:	while(<ARC>) {
	    chop;
	    ($var,@values) = split;
	    if ($var eq "Object") {
		$lface[0] = "";
		$#lface = 0;
		$lfg = "";
		$lbg = "";
		$lvis = "";
		$mm = "";
		$floor = "";
		$walkon = 0;
		$nopick = 0;
		$arch = join "_", @values;
	    }
	    if ($var eq "end") {
		if ($#lface !=0) {
		    $#lface--;
		    foreach $face (@lface) {
		        &storeFaceInfo($face, $lfg, $lbg, $lvis,$mm,$floor);
		    }
	 	}
		if ($walkon && !$nopick) {
			&warn("File $arch has an object with walk_on set which can be picked up\n");
		}
	    }
	    # Process the color/face info now
	    if ($var eq "color_fg") {
		$lfg = $values[0];
		next line;
	    }
	    if ($var eq "color_bg") {
		$lbg = $values[0];
		next line;
	    }
	    if ($var eq "visibility") {
		$lvis = $values[0];
		next line;
	    }
	    if ($var eq "magicmap") {
		$mm = $values[0];
		next line;
	    }
	    if ($var eq "is_floor") {
		$floor = $values[0];
		# is_floor is also needed for archs, so let it pass
		# through
	    }
	    elsif ($var eq "no_pick") {
		$nopick = $values[0];
	    } elsif ($var eq "walk_on") {
		$walkon = $values[0];
	    }
	    elsif ($var eq "face") {
		$lface[$#lface++] = $values[0]
	    }
	    elsif ($var eq "anim") {
		if ($anim{$arch}) {
		    &warn("$arch is a duplicate animation name");
		    $anim{$arch}="";
		}
		while (<ARC>) {
		    chomp;
		    $var = $_;
		    last if ($var =~ "mina\s*");
		    if ($var =~ /facings \S+$/) { }
		    elsif (! $faces{$var}) {
			&warn($arch." is missing face ".$var);
		    }
		    else {
			$lface[$#lface++] = $var;
		    }
		    $anim{$arch} .= "$var\n";
		}
		print ARCH "animation $arch\n";
		next line;	# don't want the mina
	    }
	    if ($var eq "face" && ! $faces{$values[0]}) {
		&warn($arch." is missing face ".$values[0])
	    }
	    print ARCH $_,"\n";
	}
	close(ARC);
    }
}

sub pline {
    local($face) = shift;
    print BMAPS sprintf("%05d",$idx++)," ",$face,"\n";
}

sub opline {
    local($face) = shift;
    print BMAPS sprintf("\\%05d",$idx++),"\t",$face,"\n";
}

sub pheader {
    print BMAPS "# This file is generated by $0, do not edit\n";
}

sub bmapsOut {
    &pheader;
    $idx = 0;
    &pline("bug.111");
    foreach $face (sort(keys %faces)) {
	&pline($face) if $face !~ /bug\.111/;
    }
}

sub pathsOut {
    &pheader;
    $idx = 0;
    &opline($root."/system/bug.111");
    foreach $face (sort(keys %faces)) {
	&opline($faces{$face}) if $faces{$face} !~ /bug\.111/;
    }
}

sub facesOut {
    foreach $face (@face_files) {
	open(FACE, $face) || &die("cannot open ".$face);
	while(<FACE>) {
	    chop;
	    local ($var, @values) = split;
	    if ($var eq "face") {
		$lface = $values[0];
		$lfg = "";
		$lbg = "";
		$lvis = "";
		$mm = "";
		$floor = "";
	    }
	    elsif ($var eq "color_fg") {
		$lfg = $values[0];
	    }
	    elsif ($var eq "color_bg") {
		$lbg = $values[0];
	    }
	    elsif ($var eq "visibility") {
		$lvis = $values[0];
	    }
	    elsif ($var eq "magicmap") {
		$mm = $values[0];
	    }
	    elsif ($var eq "is_floor") {
		$floor = $values[0];
	    }
	    elsif ($var eq "end") {
		&storeFaceInfo($lface, $lfg, $lbg, $lvis, $mm, $floor);
	    }
	    elsif ($var eq "animation") {
		$animation=$values[0];
		if ($anim{$1}) {
		    &warn("$animation is a duplicate animation name");
		    $anim{$animation}="";
		}
		while (<FACE>) {
		    chomp;
		    $var = $_;
		    last if ($var =~ /^mina\s*$/);
		    if ($var !~ /^facings/ ) {
		    	if (! $faces{$var}) {
				&warn($arch." is missing face ".$var);
			}
			else {
				$lface[$#lface++] = $var;
			}
		    }
		    $anim{$animation} .= "$var\n";
		}
		next;	# don't want the mina
	    }
	}
	close(FACE);
    }
    print FACES "# This file is generated by $0, do not edit\n";
    foreach $face (sort(keys %faces)) {
	if ($fg{$face} ne "" || $bg{$face} ne "" || $visibility{$face} ne "" ||
	    $magicmap{$face} ne "" || $floor{$face} ne "")
	{
	print FACES "face ".$face."\n";
	print FACES "color_fg ".$fg{$face}."\n"
	    if $fg{$face} ne "";
	print FACES "color_bg ".$bg{$face}."\n"
	    if $bg{$face} ne "";
	print FACES "visibility ".$visibility{$face}."\n"
	    if $visibility{$face} ne "";
	print FACES "magicmap ".$magicmap{$face}."\n"
	    if $magicmap{$face} ne "";
	print FACES "is_floor ".$floor{$face}."\n"
	    if $floor{$face} ne "";
	print FACES "end\n";
	}
    }
}

sub animOut {
    foreach $anim (sort keys %anim) {
	print ANIM "anim $anim\n$anim{$anim}mina\n";
	$animationsNum++;
    }
}

### print out statical information
sub stats {
    &info(Archs.":\t".$archsNum);
    &info(Images.":\t".$facesNum);
    &info(Faces.":\t".$facesFileNum);
    &info(Animations.":\t".$animationsNum);
    &info(Trash.":\t".$trashNum);
}
