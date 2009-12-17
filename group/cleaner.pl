#
# Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
# All rights reserved.
# This component and the accompanying materials are made available
# under the terms of the License "Eclipse Public License v1.0"
# which accompanies this distribution, and is available
# at the URL "http://www.eclipse.org/legal/epl-v10.html".
#
# Initial Contributors:
# Nokia Corporation - initial contribution.
#
# Contributors:
#
# Description:  Cleaner script to perform more thorough cleanup than abld reallyclean#

use strict;
use constant DEBUG => 0;

my $iCleanup_file = "cleanup.txt";
my @iErrors = ();
my $iTag_platform = "__PLATFORM__";
my $iTag_target = "__TARGET__";
my @iPlatforms = ( "winscw", "armv5" );
my @iTargets = ( "udeb", "urel" );
my $iSlowClean = 0;
my $iTest = 0;

# ----------------------------------------------------------------------------
# Main
# ----------------------------------------------------------------------------

my $iCommand = shift @ARGV || "";

if ( $iCommand eq "clean" ) {

    print "Running cleanup...\n";
    clean();

} elsif ( $iCommand eq "slowclean" ) {

    $iSlowClean = 1;
    clean();

} elsif ( $iCommand eq "test" ) {

    print "Running test...\n";
    $iTest = 1;
    clean();

} elsif ( $iCommand eq "init" ) {

    init_environment();

} elsif ( $iCommand eq "generate" ) {

    generate();

} else {
    print "Usage: $0 <command>\n";
    print "Commands:\n";
    print "  init      Initializes the build environment by calling 'bldmake bldfiles' and 'abld makefile'.\n";
    print "            This needs to be run only if the environment has not been built yet and 'abld build -what' fails.\n";
    print "  generate  Generates a configuration file of releasables to be cleaned.\n";
    print "  clean     Cleans the build quickly by deleting all matching files from the configuration file.\n";
    print "  slowclean Slower and more thorough clean that deletes all similar files in addition to the ones normal clean deletes.\n";
    print "            Example: Cleanup list contains file 'binary.dll'. Slowclean will also delete files named\n";
    print "            'binary.dll.foo' and 'binary.dll.bar'\n";
    print "  test      Runs a test to see what files would be deleted on a real cleanup run.\n";
    print "            Shows only files that would not be deleted by 'abld reallyclean'.\n";
    exit 0;
}

# Print possible errors
print join "\n", @iErrors;

exit 0;

# ----------------------------------------------------------------------------
# Subroutines
# ----------------------------------------------------------------------------


# ----------------------------------------------------------------------------
# Initializes the environment to be able to call 'abld build -what'
# ----------------------------------------------------------------------------
#
sub init_environment {
    # Initialize the environment in order to get "abld build -what" to run properly
    print "Initializing build environment...\n";

    print "Calling 'bldmake bldfiles'\n";
    system "bldmake bldfiles";

    print "Calling 'abld export'\n";
    system "abld export";

    print "Calling 'abld makefile winscw'\n";
    system "abld makefile winscw";

    print "Calling 'abld makefile armv5'\n";
    system "abld makefile armv5";

    print "Done.\n";
}


# ----------------------------------------------------------------------------
# Generates the cleanup list by calling 'abld build -what' and parsing it
# ----------------------------------------------------------------------------
#
sub generate {
    print "Attempting to generate the cleanup list...\n";

    # Get the list of releasables from the build tools
    open my $in, "abld build -what 2>&1 |" or die "Cannot open pipe! $!";

    my %parsed;
    while ( <$in> ) {
        if ( /No such file or directory/  ) {
            push @iErrors, "Unable to generate cleanup list. The environment is not ready. Run 'init' before running 'generate'";
            last;
        }

        $_ = lc $_;
        next unless /^\\epoc32/;

        s/release\\(?:winscw|armv5)\\(?:udeb|urel)/release\\$iTag_platform\\$iTag_target/i;

        # Cenrep file. Add the corresponding .cre file to the cleanup list
        if ( /(.+?winscw\\c\\private\\10202be9\\)(.{8})\.txt/ ) {
            $parsed{ $1 . "persists\\" . $2 . ".cre\n" }++;
        }

        $parsed{ $_ }++;
    }

    close $in;

    if ( @iErrors > 0 ) {
        return;
    }

    open my $out, ">$iCleanup_file" or die "Cannot open file $iCleanup_file! $!";
    print $out join "", sort keys %parsed;
    close $out;
}


# ----------------------------------------------------------------------------
# Cleans up the environment by deleting all files found from the cleanup list
# Cleaning is done by finding all files that match the string in the list in
# so all similarly named files are also deleted. (CodeWarrior temporary files etc )
# ----------------------------------------------------------------------------
#
sub clean {

    if ( !-e $iCleanup_file ) {
        push @iErrors, "Cleanup file $iCleanup_file not found! Run 'generate' to generate it.";
        return;
    }

    open my $in, "$iCleanup_file" or die "Cannot open cleanup file $iCleanup_file! $!";
    my @releasables = <$in>;
    close $in;

    my $total = scalar @releasables;
    print "$total rules found from cleanup list...\n";

    my @deleted = ();
    my $progress = 0;
    foreach ( @releasables ) {
        $progress++;

        next if $_ eq "";

        # Found __PLATFORM__ tag. Substitute it with all known platforms and add them back to the array
        if ( /$iTag_platform/ ) {
            foreach my $platform ( @iPlatforms ) {
                my $new_releasable = $_;
                $new_releasable =~ s/$iTag_platform/$platform/;
                debug( "Adding releasable: $new_releasable" );
                push @releasables, $new_releasable;
                $total++; # Adjust the total amount of files to be deleted
            }

            # Move on to the next round
            next;
        }

        # Found __TARGET__ tag. Substitute it with all known targets and add them back to the array
        if ( /$iTag_target/ ) {
            foreach my $target ( @iTargets ) {
                my $new_releasable = $_;
                $new_releasable =~ s/$iTag_target/$target/;
                debug( "Adding releasable: $new_releasable" );
                push @releasables, $new_releasable;
                $total++; # Adjust the total amount of files to be deleted
            }

            # Move on to the next round
            next;
        }

        # At this point there is nothing to substitute. Find all files matching the releasable name and delete them
        chomp; # Get rid of newline at the end

        my $releasable = lc $_;
        my @files_found;
        if ( $iSlowClean == 1 ) {
            @files_found = glob "$_*";
        } else {
            push @files_found, $releasable;
        }
        foreach my $file ( @files_found ) {
            next unless $file =~ /^\\epoc32/; # Some kind of safeguard, just in case

            if ( $iTest == 1 ) {

                # If the file is not one of the releasables, it would not be deleted by reallyclean
                # So add it to the list of files that would only be deleted by this script

                $file = lc $file;
                if ( $file ne $releasable || $file =~ /\.cre$/ ) {
                    push @deleted, $file;
                }

            } else {

                my $err = 0;
                unlink $file or ( $err = -1 );
                if ( $err ne -1 ) {
                    push @deleted, $file;
                }
            }
        }

        # Report progress
        my $percent = ( $progress / $total ) * 100;
        printf "\r  ( %d / %d ) % .1f %%", $progress, $total, $percent;
    }

    if ( @deleted > 0 ) {
        print "\n\nSummary:\n";
        foreach my $deleted ( sort @deleted ) {
            if ( $iTest ) {
                print "Would be deleted: $deleted\n";
            } else {
                print "Deleted: $deleted\n";
            }
        }
    } else {
        print "\n\nNothing to be done.\n";
    }
}


# ----------------------------------------------------------------------------
# Debug output. Prints a string if the debug flag is defined
# ----------------------------------------------------------------------------
#
sub debug {
    my $msg = shift;
    if ( DEBUG ) {
        print $msg;
    }
}