/*****************************************************************//**
 * \file   main.cpp
 * \brief  PoDoFo Project
 * \param  < Filename > < --(parameter) >
 * \author Oliver Bruenjes
 * \date   January 2025
 *********************************************************************/
#pragma once

#include <iostream>
#include <string>


#include "podofo.h"
#include "PDFTreeHandler.h"

void DisplayConsoleCommands()
{
    std::cout << "Usage : PDF_EXT.EXE <Filename> <Parameter>" << std::endl;
    std::cout << "Parameter: --i       displays PDF 'ZUGfERD' info" << std::endl;
    std::cout << "Parameter: --t       displays full root tree" << std::endl;
    std::cout << "Parameter: --r       displays the embedded XML file" << std::endl;
    std::cout << "Parameter: --w       write the embedded file to xml (info mode)" << std::endl;
    std::cout << "Parameter: --s       write the embedded file to xml (silet mode)" << std::endl;
    std::cout << "Parameter: --g       user guided file exploring" << std::endl;
}

int main(int argc, const char* argv[])
{
    // usage for Debug spezial file / param
    bool debug = true;

    if (debug)
    {
        argc = 3;
    }

    // check correct usage
    if (argc < 3)
    {
        DisplayConsoleCommands();
        return 1;
    }

    // try load the document
	PoDoFo::PdfMemDocument docX;
	try
	{
        if (debug)
        {
		    docX.Load(R"(C:\Projects\PDF_EXT\x64\Debug\PDF\Rechnung_000040.pdf)");
        }
        else
        {
		    docX.Load(argv[1]);
        }
	}
	catch (PoDoFo::PdfError e)
	{
		std::cerr << "ERROR: (" << e.ErrorMessage(e.GetCode()) << ")." << std::endl;
		return 1;
	}

    if (debug)
    {
        PDFTreeHandler hdlr(&docX, "Rechnung_000040.pdf");

        hdlr.UserGuidedExploration();

        return 0;
    }
    else
    {
        if (argv[2][2] == 'i')
        {
            PDFTreeHandler hdlr(&docX, argv[1]);
            hdlr.ShowPDFInfo();
            return 0;
        }
        if (argv[2][2] == 't')
        {
            PDFTreeHandler hdlr(&docX, argv[1]);
            hdlr.ShowRootTree();
            return 0;
        }
        if (argv[2][2] == 'r')
        {
            PDFTreeHandler hdlr(&docX, argv[1]);
            hdlr.ShowXMLFile();
            return 0;
        }
        if (argv[2][2] == 'w')
        {
            PDFTreeHandler hdlr(&docX, argv[1]);
            hdlr.ShowPDFInfo();
            hdlr.WriteXML(false);
            return 0;
        }
        if (argv[2][2] == 's')
        {
            PDFTreeHandler hdlr(&docX, argv[1]);
            hdlr.WriteXML(true);
            return 0;
        }
        if (argv[2][2] == 'g')
        {
            PDFTreeHandler hdlr(&docX, "Rechnung_000040.pdf");
            hdlr.UserGuidedExploration();
            return 0;
        }
    }
    // no param fit
    DisplayConsoleCommands();
    return 1;
}
