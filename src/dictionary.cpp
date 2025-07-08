#include "dictionary.h"
#include <toolbox.h>

Dictionary::Dictionary(const fs::path& filePath) : m_filepath(filePath)
{ }

// Function to parse a TEI dictionary file and search for a word
std::optional<DictionaryEntry> Dictionary::find(const std::string& searchTerm)
{
    tinyxml2::XMLDocument doc;

    // Load the XML file
    if (doc.LoadFile(m_filepath.string().c_str()) != tinyxml2::XML_SUCCESS) {
        std::cerr << "Error loading XML file: " << m_filepath << std::endl;
        return std::nullopt;
    }

    // Find the root element (usually <TEI> or <TEI.2> or similar in TEI P5)
    // Assuming the dictionary content is directly under <TEI> or <text>-><body>-><entryFree> etc.
    // We'll search directly for <entry> elements.
    tinyxml2::XMLElement* root = doc.RootElement();
    if (!root) {
        std::cerr << "Error: No root element found in XML file." << std::endl;
        return std::nullopt;
    }

    // Navigate down the hierarchy: <TEI> -> <text> -> <body>
    tinyxml2::XMLElement* textElement = root->FirstChildElement("text");
    if (!textElement) {
        std::cerr << "Warning: No <text> element found under root. Looking for <entry> directly." << std::endl;
        // Fallback: if <text> is not found, try to search for entry directly under root
        // This makes the function slightly more robust to varying TEI structures.
        // However, if your structure is consistently <text><body>, removing this fallback
        // would make it stricter.
        textElement = root; // Start search from root if <text> is missing
    }

    tinyxml2::XMLElement* bodyElement = nullptr;
    if (textElement == root) { // If <text> was not found and we defaulted to root
        bodyElement = root->FirstChildElement("body");
        if (!bodyElement) { // If <body> is also not found under root, continue with root
            bodyElement = root;
        }
    }
    else { // If <text> was found
        bodyElement = textElement->FirstChildElement("body");
        if (!bodyElement) {
            std::cerr << "Warning: No <body> element found under <text>. Looking for <entry> directly under <text>." << std::endl;
            bodyElement = textElement; // Start search for entry under <text>
        }
    }

    // Iterate through all <entry> elements
    for (tinyxml2::XMLElement* entryElement = bodyElement->FirstChildElement("entry");
        entryElement != nullptr;
        entryElement = entryElement->NextSiblingElement("entry"))
    {
        // Try to get the <form><orth> element
        tinyxml2::XMLElement* formElement = entryElement->FirstChildElement("form");
        tinyxml2::XMLElement* orthElement = nullptr;
        std::string currentLemma;

        if (formElement) {
            orthElement = formElement->FirstChildElement("orth");
            if (orthElement && orthElement->GetText()) {
                currentLemma = orthElement->GetText();
                toLowerFast(currentLemma);
            }
        }

        // Check if the current entry's lemma matches the search term (case-insensitive search)
        // You might want a more sophisticated string comparison depending on your needs.
        if (!currentLemma.empty() && currentLemma == searchTerm) {
            DictionaryEntry foundEntry;
            const char* idAttr = entryElement->Attribute("xml:id");
            if (idAttr) {
                foundEntry.id = idAttr;
            }
            else {
                foundEntry.id = currentLemma; // Fallback if no xml:id
            }
            foundEntry.lemma = currentLemma;

            // Parse senses
            for (tinyxml2::XMLElement* senseElement = entryElement->FirstChildElement("sense");
                senseElement != nullptr;
                senseElement = senseElement->NextSiblingElement("sense"))
            {
                Sense currentSense;
                const char* senseNumAttr = senseElement->Attribute("n");
                if (senseNumAttr) {
                    try {
                        currentSense.number = std::stoi(senseNumAttr);
                    }
                    catch (const std::invalid_argument& e) {
                        std::cerr << "Warning: Invalid sense number '" << senseNumAttr << "' for entry '" << currentLemma << "'." << std::endl;
                    }
                    catch (const std::out_of_range& e) {
                        std::cerr << "Warning: Sense number out of range for entry '" << currentLemma << "'." << std::endl;
                    }
                }

                // Parse cit
                for (tinyxml2::XMLElement* citElement = senseElement->FirstChildElement("cit");
                    citElement != nullptr;
                    citElement = citElement->NextSiblingElement("sense")) {
                    tinyxml2::XMLElement* quoteElement = citElement->FirstChildElement("quote");
                    if (quoteElement && quoteElement->GetText()) {
                        Cit cit;
                        cit.quote = quoteElement->GetText();
                        currentSense.cits.push_back(cit);
                    }
                }

                // Parse grammatical group
                tinyxml2::XMLElement* gramGrpElement = senseElement->FirstChildElement("gramGrp");
                if (gramGrpElement) {
                    tinyxml2::XMLElement* posElement = gramGrpElement->FirstChildElement("pos");
                    if (posElement && posElement->GetText()) {
                        currentSense.gramGrp.partOfSpeech = posElement->GetText();
                    }
                }

                // Parse definition
                tinyxml2::XMLElement* defElement = senseElement->FirstChildElement("def");
                if (defElement && defElement->GetText()) {
                    currentSense.definition = defElement->GetText();
                }

                // Parse examples
                for (tinyxml2::XMLElement* exampleElement = senseElement->FirstChildElement("example");
                    exampleElement != nullptr;
                    exampleElement = exampleElement->NextSiblingElement("example"))
                {
                    tinyxml2::XMLElement* egElement = exampleElement->FirstChildElement("eg");
                    if (egElement && egElement->GetText()) {
                        Example currentExample;
                        currentExample.text = egElement->GetText();
                        currentSense.examples.push_back(currentExample);
                    }
                }
                foundEntry.senses.push_back(currentSense);
            }
            return foundEntry; // Found and parsed, return it
        }
    }

    return std::nullopt; // Word not found
}

// Helper function to print a dictionary entry
void DictionaryEntry::print() const {
    std::cout << "------------------------------------" << std::endl;
    std::cout << "Lemma (ID: " << id << "): " << lemma << std::endl;
    for (const auto& sense : senses) {
        std::cout << "  Sense " << sense.number << ":" << std::endl;
        if (!sense.gramGrp.partOfSpeech.empty()) {
            std::cout << "    Part of Speech: " << sense.gramGrp.partOfSpeech << std::endl;
        }
        std::cout << "    Definition: " << sense.definition << std::endl;
        for (const auto& example : sense.examples) {
            std::cout << "    Example: " << example.text << std::endl;
        }
    }
    std::cout << "------------------------------------" << std::endl;
}
