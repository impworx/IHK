#include "PDFTreeHandler.h"

PDFTreeHandler::PDFTreeHandler(PoDoFo::PdfMemDocument* doc, std::string fileName) : m_doc(doc), m_FileName(fileName)
{
    // init file
    InitializePDFFile(&m_doc->GetTrailer().GetObject());

}

void PDFTreeHandler::InitializePDFFile(const PoDoFo::PdfObject* obj)
{
    if (!obj)
    {
        return;
    }
    if (obj->IsDictionary())
    {
        for (auto& [key, value] : obj->GetDictionary())
        {
            InitializePDFFile(&value);
        }
    }
    else if (obj->IsArray())
    {
        try
        {
            const PoDoFo::PdfArray& array = obj->GetArray();
            for (auto& elem : array)
            {
                if (elem.IsReference())
                {
                    if (array.GetOwner()->GetDocument()->GetObjects().GetObject(elem.GetReference())->HasStream())
                    {
                        CheckRefObjStreamData(array.GetOwner()->GetDocument()->GetObjects().GetObject(elem.GetReference()));
                    }

                    if (!IsObjectInColl(elem.GetReference().ObjectNumber()))
                    {
                        AddObject(array.GetOwner()->GetDocument()->GetObjects().GetObject(elem.GetReference()), elem.GetReference().ObjectNumber());
                        InitializePDFFile(array.GetOwner()->GetDocument()->GetObjects().GetObject(elem.GetReference()));
                    }
                }
                else
                {
                    InitializePDFFile(&elem);
                }
            }
        }
        catch (PoDoFo::PdfError e)
        {
            std::cerr << "Array Error : " << e.ErrorName(e.GetCode()) << " - " << e.ErrorMessage(e.GetCode()) << std::endl;
            std::exit(-1);
        }
    }
    else if (obj->IsString())
    {
        if (obj->GetString().GetString().find(m_XMLFileSearchVal) != obj->GetString().GetString().npos && m_hasFoundXML == false)
        {
            m_XMLFileName = obj->GetString();
            m_hasFoundXML = true;
        }
        // TODO -> Handle more .xml Filenames??
    }
    else if (obj->IsReference())
    {
        if (obj->GetParent()->GetOwner()->GetDocument()->GetObjects().GetObject(obj->GetReference())->HasStream())
        {
            CheckRefObjStreamData(obj->GetParent()->GetOwner()->GetDocument()->GetObjects().GetObject(obj->GetReference()));
        }

        // TODO -> If Ref hast Stream check xml and store if true for more files?

        if (!IsObjectInColl(obj->GetReference().ObjectNumber()))
        {
            AddObject(obj->GetParent()->GetOwner()->GetDocument()->GetObjects().GetObject(obj->GetReference()), obj->GetReference().ObjectNumber());
            InitializePDFFile(obj->GetParent()->GetOwner()->GetDocument()->GetObjects().GetObject(obj->GetReference()));
        }
    }
}

// ==== Helper methods

void PDFTreeHandler::CheckRefObjStreamData(PoDoFo::PdfObject* obj)
{
    if (m_proofedXMLData == false) {
        m_Buffer.clear();
        m_Buffer = obj->GetStream()->GetCopy();
        if (m_Buffer.find(m_XMLSearchValue) != m_Buffer.npos)
        {
            m_EmbeddedFile = obj;
            m_proofedXMLData = true;
        }
    }
}

void PDFTreeHandler::CreateOutputFileName()
{
    std::string tmpStr = m_FileName;
    tmpStr.pop_back();
    tmpStr.pop_back();
    tmpStr.pop_back();
    tmpStr.pop_back();
    tmpStr.append(m_XMLFileSearchVal);
    m_OutputFileName = tmpStr;
}

void PDFTreeHandler::WriteXMLToFile()
{
    std::ofstream oFile(m_OutputFileName, std::ios::binary);
    oFile.write(m_Buffer.data(), std::streamsize(m_Buffer.size()));
    oFile.close();
}

// ==== Display methods ====

void PDFTreeHandler::TraversePdfObject(const PoDoFo::PdfObject* obj, std::string indent)
{
    if (!obj)
    {
        std::cout << indent << "(null)" << std::endl;
        return;
    }
    if (obj->IsDictionary())
    {
        std::cout << indent << "Dictionary {" << std::endl;
        for (auto& [key, value] : obj->GetDictionary())
        {
            std::cout << indent << "   Key: " << key.GetString() << std::endl;
            TraversePdfObject(&value, indent + "     ");

        }
        std::cout << indent << "}" << std::endl;
    }
    else if (obj->IsArray())
    {
        try
        {
            const PoDoFo::PdfArray& array = obj->GetArray();
            std::cout << indent << "Array (" << array.GetSize() << ") - [" << std::endl;
            for (auto& elem : array)
            {
                if (elem.IsReference())
                {
                    if (!IsObjectInColl(elem.GetReference().ObjectNumber())) 
                    {
                        AddObject(array.GetOwner()->GetDocument()->GetObjects().GetObject(elem.GetReference()), elem.GetReference().ObjectNumber());
                        TraversePdfObject(array.GetOwner()->GetDocument()->GetObjects().GetObject(elem.GetReference()), indent + "     ");
                    }
                    else
                    {
                        // display Reference anyway
                        std::cout << indent << "Reference : " << elem.GetReference().ToString() << std::endl;
                        std::cout << indent << "Reference has datastream (" << (array.GetOwner()->GetDocument()->GetObjects().GetObject(elem.GetReference())->HasStream() ? "true)" : "false)") << std::endl;
                    }
                }
                else
                {
                    TraversePdfObject(&elem, indent + "     ");
                }
            }
            std::cout << indent << "]" << std::endl;
        }
        catch (PoDoFo::PdfError e)
        {
            std::cerr << "Array Error : " << e.ErrorName(e.GetCode()) << " - " << e.ErrorMessage(e.GetCode()) << std::endl;
            return;
        }
    }
    else if (obj->HasStream())
    {
        std::cout << indent << "Stream: (length: " << obj->GetStream()->GetLength() << ")" << std::endl;
    }
    else if (obj->IsRawData())
    {
        std::cout << indent << "Raw Data found." << std::endl;
    }
    else if (obj->IsString())
    {
        std::cout << indent << "String: " << obj->GetString().GetString() << std::endl;
    }
    else if (obj->IsNumber())
    {
        std::cout << indent << "Number: " << obj->GetNumber() << std::endl;
    }
    else if (obj->IsBool())
    {
        std::cout << indent << "Boolean: " << (obj->GetBool() ? "true" : "false") << std::endl;
    }
    else if (obj->IsReference())
    {
        std::cout << indent << "Reference: " << obj->GetReference().ToString() << std::endl;
        std::cout << indent << "Reference has datastream (" << (obj->GetParent()->GetOwner()->GetDocument()->GetObjects().GetObject(obj->GetReference())->HasStream() ? "true)" : "false)") << std::endl;
        
        if (!IsObjectInColl(obj->GetReference().ObjectNumber()))
        {
            AddObject(obj->GetParent()->GetOwner()->GetDocument()->GetObjects().GetObject(obj->GetReference()), obj->GetReference().ObjectNumber());
            TraversePdfObject(obj->GetParent()->GetOwner()->GetDocument()->GetObjects().GetObject(obj->GetReference()), indent + "     ");
        }
        else
        {
           // std::cout << indent << "Reference : " << obj->GetReference().ToString() << std::endl;
        }
    }
    else if (obj->IsNull())
    {
        std::cout << indent << "NULL" << std::endl;
    }
    else
    {
        std::cout << indent << "Unknown type!" << std::endl;
    }
}

void PDFTreeHandler::ShowXMLFile(int ref)
{
    // set the locale to display correct ä ü ö
    std::setlocale(LC_ALL, "de_DE.UTF-8");
    if (ref == -1)
    {
        std::cout.write(m_Buffer.data(), std::streamsize(m_Buffer.size()));
    }
    else
    {
        std::cout.write(GetObject(ref)->GetStream()->GetCopy().data(), std::streamsize(GetObject(ref)->GetStream()->GetCopy().size()));
    }
}

void PDFTreeHandler::ShowRootTree()
{
    // clear table cause to display and add referenced objects once!
    m_objTable.clear();
    TraversePdfObject(&m_doc->GetTrailer().GetObject());
}

void PDFTreeHandler::WriteXML(bool silenced)
{
    CreateOutputFileName();
    WriteXMLToFile();
    if (!silenced)
    {
        std::cout << "File : " << m_OutputFileName << " written successfully." << std::endl;
    }
}

void PDFTreeHandler::UserGuidedExploration()
{
    bool is_running = true;
    std::string input;
    //  Show Info
    
    ShowPDFInfo();

    std::cout << "\n" << "\n" << std::endl;
    // Rollout
    
    m_TreePos.push_back(&m_doc->GetTrailer().GetObject());
    TraversePdfObject(&m_doc->GetTrailer().GetObject(), "---");

    // Navigate 
    
    std::cout << "**********************************************************" << std::endl;
    std::cout << "type ref number ( i.e. '1' ) to navigate through PDF tree" << std::endl;
    std::cout << "type ( 0 ) to navigate back.." << std::endl;
    std::cout << "type ( exit ) to quit this programm..\n" << std::endl;
    
    int objRef = -1;

    while (is_running)
    {
        // navigation command
        std::cout << "Command : ";
        std::getline(std::cin, input);

        try
        {
            if (input.substr(0, 5) != "print")
                objRef = std::stoi(input); //// cant get int value!!!
        }
        catch (std::invalid_argument a)
        {

        }

        if (objRef > 0)
        {
            // find ref
            if (IsObjectInColl(objRef))
            {
                m_TreePos.push_back(GetObject(objRef));
                objRef = -1;
            }
            else
            {
                std::cout << "\nObject referenced to : " << objRef << " not found.\n" << std::endl;
                objRef = -1;
            }
        }

        if (input == "0")
        {
            if (m_TreePos.size() < 2)
            {
                std::cout << "\nRoot entry already reached!\n" << std::endl;
            } 
            else
            {
                m_TreePos.pop_back();
            }

        }

        if (input.substr(0, 5) == "print")
        {
            int ref = std::stoi(input.substr(6));
            std::cout << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n";
            std::cout << "XXXXXXXXXX         SHOW DATA STREAM         XXXXXXXXXXXXXXXX\n";
            std::cout << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n\n";
            if (GetObject(ref)->HasStream())
                ShowXMLFile(ref);
            std::cout << "\n\nXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n\n";

        }

        if (input == "exit")
        {
            is_running = !is_running;
            exit;
        }

        std::cout << "**********************************************************\n" << std::endl;


        std::string space = "";
        for (auto elem : m_TreePos)
        {
            std::cout << "|------------------> Object number :" << elem->GetIndirectReference().ObjectNumber() << std::endl;
            TraversePdfObject(elem, space);
            space += "------>||";
        }


        std::cout << "**********************************************************\n" << std::endl;
        std::cout << "type ref number ( i.e. '1' ) to navigate through PDF tree" << std::endl;
        std::cout << "type ( 0 ) to navigate back.." << std::endl;
        std::cout << "type ( print <ref number> ) to display stream data..\n" << std::endl;
        std::cout << "type ( exit ) to quit this programm..\n" << std::endl;


    }
}

void PDFTreeHandler::ShowPDFInfo()
{
    std::cout << "Filename :              " << m_FileName << std::endl;
    std::cout << "References with stream :" << CountObjectsWithStream() << std::endl;
    std::cout << "XML file name found? :  " << (m_hasFoundXML == 1 ? "True" : "False") << std::endl;
    std::cout << "XML file name :         " << m_XMLFileName.GetString() << std::endl;
    std::cout << "Object stream :         " << (m_EmbeddedFile ? (m_EmbeddedFile->HasStream() == 1 ? "True" : "False") : "") << std::endl;
    std::cout << "Stream XML proofed? :   " << (m_proofedXMLData == 1 ? "True" : "False") << std::endl;
}

// ==== Handler for referenzed PDF Objects ====

void PDFTreeHandler::AddObject(PoDoFo::PdfObject *obj, size_t ref)
{
	m_objTable[ref] = std::ref(obj);
}

PoDoFo::PdfObject* PDFTreeHandler::GetObject(size_t ref)
{
	return m_objTable[ref];
}

bool PDFTreeHandler::IsObjectInColl(size_t ref)
{
    if (m_objTable.find(ref) == m_objTable.end())
    { 
        return false;
    }
    return true;
}

size_t PDFTreeHandler::CountObjectsWithStream()
{
    size_t counter = 0;

    for (auto elem : m_objTable)
    {
        if (elem.second->HasStream())
        {
            counter++;
        }
    }
    return counter;
}

