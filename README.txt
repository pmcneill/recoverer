Recoverer
version 0.0.1
Patrick McNeill (pmcneill@gmail.com)

Recoverer is a filesystem-agnostic file recovery tool.  Meant to recover
files from corrupted memory cards, it works by first creating a
"fingerprint" to find the files you'd like to recover, then running the
actual recovery program to restore them.

The first step of this process involves running "fingerprint" with any
number of filenames as arguments.  All of the files should be of the same
type, but be different in content.  fingerprint will use these files to 
determine what parts of the beginning and end (by default, 32 bytes on each
end) are common to all files.  You will then be asked to name this filetype,
give a file extension for it, and give a minimum and maximum file size (to
reduce search time plus to detect files with no end).

The second step is to actually perform the recovery operation.  To do this,
run "recoverer" with two parameters: the first is the fingerprint file from
step 1, the second is the file (or disk) to recover from.  It will create
the files sequentially in the current directory, beginning with 0000.XXX, where
XXX is the extension you chose in the fingerprint file.
