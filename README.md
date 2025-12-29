# drawing-tui
Drawing TUI based on ncurses for linux, This program serves as a canvas for making large or small asci art with an intuitive interface.

Its features are as follows:
- Selecting any ASCII character for drawing.
- Using a string instead of a single character for drawing.
- Saving/loading a drawing.
- Converting the saved drawing file (.curse extenstion) into a file, such that its contents are interpreted as colored art for a xterm complying terminal.
- Choosing any color for the text or background, using a hex value.

## The translator
The translator creates a file named BashCodedFile, that can be interpreted by bash to be colored. To do this, run this command on your saved drawing:

`./translator YOUR_FILE.curse`

This will generate the BashCodedFile in your current directory, to view the drawing simply run:

``echo -e "`cat BasCodedFile`"``

## Dependencies
- ncurses v6.1 or higher
- gcc compilers and standard C libraries

## Installation
Clone this repo, then with Make, simply run:

`make all`

Or for the drawing-tui and the translator only respectively:

`make build`

`make translator`

## To-do
- ☐ Improve loading saved drawings of different dimensions.
- ☐ Configure switchable vim-style navigation option.
- ☐ Make a menu for seeing current and previously used paletettes.
- ☐ Make a Windows version for PowerShell.
- ✅ Make it so saved pictures are loaded with their colors.
- ✅ Make a translator for the custom file used to save drawings to bash (hopefully with color).
