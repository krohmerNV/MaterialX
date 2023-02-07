#!/usr/bin/python

import sys
import os
import datetime
import argparse

try:
    # Use pip to install Pillow and Image to enable image diffs
    from PIL import Image, ImageChops
    DIFF_ENABLED = True
except Exception:
    DIFF_ENABLED = False

def createDiff(image1Path, image2Path, imageDiffPath):
    try:
        if os.path.exists(imageDiffPath):
            os.remove(imageDiffPath)
        
        if not os.path.exists(image1Path):
            print ("Image diff input missing: " + image1Path)
            return
        
        if not os.path.exists(image2Path):
            print ("Image diff input missing: " + image2Path)
            return

        image1 = Image.open(image1Path).convert('RGB')
        image2 = Image.open(image2Path).convert('RGB')
        diff = ImageChops.difference(image1, image2)
        diff.save(imageDiffPath)
    except Exception:
        if os.path.exists(imageDiffPath):
            os.remove(imageDiffPath)
        print ("Failed to create image diff between: " + image1Path + ", " + image2Path)

def main(args=None):

    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--inputdir', dest='inputdir', action='store', help='Input directory', default=".")
    parser.add_argument('-i2', '--inputdir2', dest='inputdir2', action='store', help='Second input directory', default=".")
    parser.add_argument('-i3', '--inputdir3', dest='inputdir3', action='store', help='Third input directory', default=".")
    parser.add_argument('-o', '--outputfile', dest='outputfile', action='store', help='Output file name', default="tests.html")
    parser.add_argument('-d', '--diff', dest='CREATE_DIFF', action='store_true', help='Perform image diff', default=False)
    parser.add_argument('-t', '--timestamp', dest='ENABLE_TIMESTAMPS', action='store_true', help='Write image timestamps', default=False)
    parser.add_argument('-w', '--imagewidth', type=int, dest='imagewidth', action='store', help='Set image display width', default=256)
    parser.add_argument('-ht', '--imageheight', type=int, dest='imageheight', action='store', help='Set image display height', default=256)
    parser.add_argument('-cp', '--cellpadding', type=int, dest='cellpadding', action='store', help='Set table cell padding', default=0)
    parser.add_argument('-tb', '--tableborder', type=int, dest='tableborder', action='store', help='Table border width. 0 means no border', default=3)
    parser.add_argument('-sl', '--sourcelang', dest='sourcelang', action='store', help='Source language. Default is glsl', default="glsl")
    parser.add_argument('-dl', '--destlang', dest='destlang', action='store', help='Destination language. Default is osl', default="osl")
    parser.add_argument('-dl2', '--destlang2', dest='destlang2', action='store', help='Second destination language. Default is empty', default="")

    args = parser.parse_args(args)

    fh = open(args.outputfile,"w+")
    fh.write("<html>\n")
    fh.write("<style>\n")
    fh.write("td {")
    fh.write("    padding: " + str(args.cellpadding) + ";")
    fh.write("    border: " + str(args.tableborder) + "px solid black;")
    fh.write("}")
    fh.write("table, tbody, th, .td_image {")
    fh.write("    border-collapse: collapse;")
    fh.write("    padding: 0;")
    fh.write("    margin: 0;")
    fh.write("}")
    fh.write("</style>")
    fh.write("<body>\n")
    dir1 = os.getcwd() if args.inputdir == "." else args.inputdir
    dir2 = os.getcwd() if args.inputdir2 == "." else args.inputdir2
    dir3 = os.getcwd() if args.inputdir3 == "." else args.inputdir3
    
    useThirdLang = True if args.destlang2 and (args.inputdir != args.inputdir3 or args.sourcelang != args.destlang2) else False
    
    if useThirdLang:
        fh.write("<h3>" + args.sourcelang + " (in: " + dir1 + ") vs "+ args.destlang + " (in: " + dir2 + ") vs "+ args.destlang2 + " (in: " + dir3 + ")</h3>\n")
    else:
        fh.write("<h3>" + args.sourcelang + " (in: " + dir1 + ") vs "+ args.destlang + " (in: " + dir2 + ")</h3>\n")

    if not DIFF_ENABLED and args.CREATE_DIFF:
        print("--diff argument ignored. Diff utility not installed.")

    if not args.inputdir2:
        args.inputdir2 = args.inputdir
    if useThirdLang and not args.inputdir3:
        args.inputdir3 = args.inputdir

    # Get all source files
    sourceFiles = []
    sourcePaths = []
    for subdir, _, files in os.walk(args.inputdir):
        for curFile in files:
            if curFile.endswith(args.sourcelang + ".png"):
                sourceFiles.append(curFile)
                sourcePaths.append(subdir) 

    # Get all destination files, matching source files
    destFiles = []
    destPaths = []
    destFiles2 = []
    destPaths2 = []
    postFix = args.sourcelang + ".png"
    for sourceFile, sourcePath in zip(sourceFiles, sourcePaths):
        # Allow for just one language to be shown if source and dest are the same.
        # Otherwise add in equivalent name with dest language replacement if
        # pointing to the same directory 
        if args.inputdir != args.inputdir2 or args.sourcelang != args.destlang:
            destFile = sourceFile[:-len(postFix)] + args.destlang + ".png"
            destPath = os.path.join(args.inputdir2, sourcePath)
        else:
            destFile = ""
            destPath = None
        destFiles.append(destFile)
        destPaths.append(destPath)

        if useThirdLang:
            destFile = sourceFile[:-len(postFix)] + args.destlang2 + ".png"
            destPath = os.path.join(args.inputdir2, sourcePath)
        else:
            destFile = ""
            destPath = None
        destFiles2.append(destFile)
        destPaths2.append(destPath)

    if sourceFiles:
        curPath = ""
        for sourceFile, destFile, destFile2, sourcePath, destPath, destPath2 in zip(sourceFiles, destFiles, destFiles2, sourcePaths, destPaths, destPaths2):

            fullSourcePath = os.path.join(sourcePath, sourceFile) if sourceFile else None
            fullDestPath = os.path.join(destPath, destFile) if destFile else None
            fullDestPath2 = os.path.join(destPath2, destFile2) if destFile2 else None

            if curPath != sourcePath:
                if curPath != "":
                    fh.write("</table>\n")
                fh.write("<p>" + os.path.normpath(sourcePath) + ":</p>\n")
                fh.write("<table>\n")
                curPath = sourcePath

            if sourceFile and destFile and DIFF_ENABLED and args.CREATE_DIFF:
                diffPath = fullSourcePath[0:-8] + "_" + args.sourcelang + "_vs_" + args.destlang + "_diff.png"
                createDiff(fullSourcePath, fullDestPath, diffPath)
            else:
                diffPath = None

            if useThirdLang and sourceFile and destFile2 and DIFF_ENABLED and args.CREATE_DIFF:
                diffPath2 = fullSourcePath[0:-8] + "_" + args.sourcelang + "_vs_" + args.destlang2 + "_diff.png"
                createDiff(fullSourcePath, fullDestPath2, diffPath2)
                diffPath3 = fullSourcePath[0:-8] + "_" + args.destlang + "_vs_" + args.destlang2 + "_diff.png"
                createDiff(fullDestPath, fullDestPath2, diffPath3)
            else:
                diffPath2 = None

            if os.path.isabs(args.outputfile):
                fileUri = 'file:///'
            else:
                fileUri =''

            fh.write("<tr>\n")
            if fullSourcePath:
                fh.write("<td class='td_image'><img src='" + fileUri + fullSourcePath + "' height='" + str(args.imageheight) + "' width='" + str(args.imagewidth) + "' loading='lazy' style='background-color:black;'/></td>\n")
            if fullDestPath:
                fh.write("<td class='td_image'><img src='" + fileUri + fullDestPath + "' height='" + str(args.imageheight) + "' width='" + str(args.imagewidth) + "' loading='lazy' style='background-color:black;'/></td>\n")
            if fullDestPath2:
                fh.write("<td class='td_image'><img src='" + fileUri + fullDestPath2 + "' height='" + str(args.imageheight) + "' width='" + str(args.imagewidth) + "' loading='lazy' style='background-color:black;'/></td>\n")
            if diffPath:
                fh.write("<td class='td_image'><img src='" + fileUri + diffPath + "' height='" + str(args.imageheight) + "' width='" + str(args.imagewidth) + "' loading='lazy' style='background-color:black;'/></td>\n")
            if diffPath2:
                fh.write("<td class='td_image'><img src='" + fileUri + diffPath2 + "' height='" + str(args.imageheight) + "' width='" + str(args.imagewidth) + "' loading='lazy' style='background-color:black;'/></td>\n")
            if diffPath3:
                fh.write("<td class='td_image'><img src='" + fileUri + diffPath3 + "' height='" + str(args.imageheight) + "' width='" + str(args.imagewidth) + "' loading='lazy' style='background-color:black;'/></td>\n")
            fh.write("</tr>\n")

            fh.write("<tr>\n")
            if fullSourcePath:
                fh.write("<td align='center'>" + sourceFile)
            if args.ENABLE_TIMESTAMPS and os.path.isfile(fullSourcePath):
                fh.write("<br>(" + str(datetime.datetime.fromtimestamp(os.path.getmtime(fullSourcePath))) + ")")
            fh.write("</td>\n")
            if fullDestPath:
                fh.write("<td align='center'>" + destFile)
            if args.ENABLE_TIMESTAMPS and os.path.isfile(fullDestPath):
                fh.write("<br>(" + str(datetime.datetime.fromtimestamp(os.path.getmtime(fullDestPath))) + ")")
            fh.write("</td>\n")
            if fullDestPath2:
                fh.write("<td align='center'>" + destFile2)
            if args.ENABLE_TIMESTAMPS and os.path.isfile(fullDestPath2):
                fh.write("<br>(" + str(datetime.datetime.fromtimestamp(os.path.getmtime(fullDestPath2))) + ")")
            fh.write("</td>\n")
            if diffPath:
                fh.write("<td align='center'>Difference " + args.sourcelang + " vs. " + args.destlang + " </td>\n")
            if diffPath2:
                fh.write("<td align='center'>Difference " + args.sourcelang + " vs. " + args.destlang2 + " </td>\n")
            if diffPath3:
                fh.write("<td align='center'>Difference " + args.destlang + " vs. " + args.destlang2 + " </td>\n")
            fh.write("</tr>\n")

    fh.write("</table>\n")
    fh.write("</body>\n")
    fh.write("</html>\n")

if __name__ == "__main__":
    main(sys.argv[1:])
