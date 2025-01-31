/*****************************************************************//**
 * \file   PDFTreeHandler.h
 * \brief  Helper class to handle PDF usages
 * 
 * \author Oliver Bruenjes
 * \date   January 2025
 *********************************************************************/
#pragma once
#include "podofo.h"

#include <unordered_map>

#include <iostream>
#include <fstream>


class PDFTreeHandler
{
public:
	/**
	 * Standart constructor.
	 */
	PDFTreeHandler() {};
	/**
	 * PDFTreeHandler constructor.
	 * 
	 * \param doc
	 * \param fileName
	 */
	PDFTreeHandler(PoDoFo::PdfMemDocument *doc, std::string fileName);

public:

	// ==== Display methods ====

	/**
	 * Shows the ZUGfERD relevant PDF infos.
	 * 
	 */
	void ShowPDFInfo();
	/**
	 * Shows the embedded XML file if found.
	 * 
	 */
	void ShowXMLFile(int ref = -1);
	/**
	 * Shows the hole PDF Tree with all Objects.
	 * 
	 */
	void ShowRootTree();

	/**
	 * Writes the XML Data to <Filename>.xml.
	 * 
	 * \param silenced | false = with console log output.
	 */
	void WriteXML(bool silenced);

	void UserGuidedExploration();

private:

	// ==== Init Method(s)

	/**
	 * Initialize the PDF document includes searching for XML embedded file(s).
	 * 
	 * \param obj
	 */
	void InitializePDFFile(const PoDoFo::PdfObject* obj);

	// ==== Helper method(s)

	/**
	 * Displays the hole ROOT tree structure.
	 * 
	 * \param obj
	 * \param indent | Tree structure string
	 */
	void TraversePdfObject(const PoDoFo::PdfObject* obj, std::string indent = std::string(""));
	/**
	 * Checks the found stream data for XML content.
	 * 
	 * \param obj
	 */
	void CheckRefObjStreamData(PoDoFo::PdfObject* obj);
	/**
	 * Convert Filename.pdf to Filename.xml.
	 * 
	 */
	void CreateOutputFileName();
	/**
	 * Simply writes the XML stream data to file if data exists.
	 * 
	 */
	void WriteXMLToFile();

	// ==== Referenced object handling ====

	/**
	 * Add a referenced object to the list.
	 * 
	 * \param obj
	 * \param ref | Object reference number
	 */
	void AddObject(PoDoFo::PdfObject *obj, size_t ref);
	/**
	 * Gets the referenced object by object number.
	 * 
	 * \param ref | Object number.
	 * \return | Returns the object if this exits in list
	 */
	PoDoFo::PdfObject* GetObject(size_t ref);
	/**
	 * Get true if the object with the given object number is in the list.
	 * 
	 * \param ref | Object reference number.
	 * \return | true / false
	 */
	bool IsObjectInColl(size_t ref);
	/**
	 * Counts all referenced objects in list which have stream data.
	 * 
	 * \return | count (size_t)
	 */
	size_t CountObjectsWithStream();
	


private:

	const std::string m_XMLFileSearchVal = ".xml";
	const std::string m_XMLSearchValue = "<?xml version";

	std::string m_FileName;
	std::string m_OutputFileName;

	// Document
	const PoDoFo::PdfMemDocument *m_doc = nullptr;

	// Ref Obj Storage
	std::unordered_map<size_t, PoDoFo::PdfObject*> m_objTable;

	// === ZUGfERD Information ===
	// Stream Data
	PoDoFo::PdfObjectStream* m_ObjStream = nullptr;
	// XML handling
	bool m_hasFoundXML = false;
	PoDoFo::PdfString m_XMLFileName;		// found .xml name
	bool m_proofedXMLData = false;		// found '<?xml version'
	PoDoFo::PdfObject* m_EmbeddedFile = nullptr; // this object holds the ebedded file (streamproofed)
	PoDoFo::charbuff m_Buffer;

	// Positioning Vector
	std::vector<const PoDoFo::PdfObject*> m_TreePos;
};

