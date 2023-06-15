/*! \file
    \brief Утилита umba-tr - сборка единого файла перевода из дерева каталогов/файлов перевода
 */

// Должна быть первой
#include "umba/umba.h"
//---

//#-sort
#include "umba/simple_formatter.h"
#include "umba/char_writers.h"
//#+sort

#include "umba/debug_helpers.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <filesystem>

#include "umba/debug_helpers.h"
#include "umba/string_plus.h"
#include "umba/program_location.h"
#include "umba/scope_exec.h"
#include "umba/macro_helpers.h"
#include "umba/macros.h"

#include "marty_cpp/marty_cpp.h"
#include "marty_cpp/sort_includes.h"
#include "marty_cpp/enums.h"
#include "marty_cpp/src_normalization.h"

#include "encoding/encoding.h"
#include "umba/cli_tool_helpers.h"
#include "umba/time_service.h"
#include "umba/scanners.h"
#include "umba/filesys.h"
#include "umba/filename.h"
#include "umba/format_message.h"

#include "marty_tr/enums_decl.h"
#include "marty_tr/enums_serialization.h"
#include "marty_tr/marty_tr.h"
#include "marty_tr/locales.h"

#include "encoding/encoding.h"


#if defined(WIN32) || defined(_WIN32)

    #define HAS_CLIPBOARD_SUPPORT 1
    #include "umba/clipboard_win32.h"

#endif


#include "app_config.h"


// #include "umba/time_service.h"
// #include "umba/text_utils.h"



umba::StdStreamCharWriter coutWriter(std::cout);
umba::StdStreamCharWriter cerrWriter(std::cerr);
umba::NulCharWriter       nulWriter;

umba::SimpleFormatter umbaLogStreamErr(&cerrWriter);
umba::SimpleFormatter umbaLogStreamMsg(&cerrWriter);
umba::SimpleFormatter umbaLogStreamNul(&nulWriter);

bool umbaLogGccFormat   = false; // true;
bool umbaLogSourceInfo  = false;

bool bOverwrite    = false;
bool bForce        = false;

std::string               outputFilename;
std::string               substCategoryName;
marty_tr::ELangTagFormat  langTagFormat = marty_tr::ELangTagFormat::langIdFull; // langId;
//bool        substSategory = false;

using marty_cpp::ELinefeedType;

#if defined(WIN32) || defined(_WIN32)

    ELinefeedType outputLinefeed = ELinefeedType::crlf;

#else

    // Linups and so
    ELinefeedType outputLinefeed = ELinefeedType::lf;

#endif


// bool useClipboard


#include "log.h"

// umba::program_location::ProgramLocation<std::string>   programLocationInfo;


#include "umba/cmd_line.h"

//

AppConfig appConfig;

// Конфиг версии
#include "app_ver_config.h"
// Принтуем версию
#include "print_ver.h"
// Парсер параметров командной строки
#include "arg_parser.h"



int main(int argc, char* argv[])
{

    using namespace umba::omanip;


    auto argsParser = umba::command_line::makeArgsParser( ArgParser()
                                                        , CommandLineOptionCollector()
                                                        , argc, argv
                                                        , umba::program_location::getProgramLocation
                                                            ( argc, argv
                                                            , false // useUserFolder = false
                                                            //, "" // overrideExeName
                                                            )
                                                        );

    // Force set CLI arguments while running under debugger
    if (umba::isDebuggerPresent())
    {
        argsParser.args.clear();
        argsParser.args.push_back("@../umba-tr.rsp");
        argsParser.args.push_back("../test-tr.json");

        //argsParser.args.push_back("@..\\make_sources_brief.rsp");
        // argsParser.args.push_back(umba::string_plus::make_string(""));
        // argsParser.args.push_back(umba::string_plus::make_string(""));
        // argsParser.args.push_back(umba::string_plus::make_string(""));
    }

    //programLocationInfo = argsParser.programLocationInfo;

    // try
    // {
        // Job completed - may be, --where option found
        if (argsParser.mustExit)
            return 0;
       
        // if (!argsParser.quet)
        // {
        //     printNameVersion();
        // }
       
        if (!argsParser.parseStdBuiltins())
            return 1;
        if (argsParser.mustExit)
            return 0;
       
        if (!argsParser.parse())
            return 1;
        if (argsParser.mustExit)
            return 0;
    // }
    // catch(const std::exception &e)
    // {
    //     LOG_ERR_OPT << e.what() << "\n";
    //     return -1;
    // }
    // catch(const std::exception &e)
    // {
    //     LOG_ERR_OPT << "command line arguments parsing error" << "\n";
    //     return -1;
    // }


    if (appConfig.includeFilesMaskList.empty() && appConfig.excludeFilesMaskList.empty())
    {
        if (!appConfig.allFiles)
        {
            LOG_ERR_OPT << "no --exclude-files nor --include-files mask are taken, requires --all to confirm processing all files"
                        << "\n";
            return 1;
        }
    }

    // appConfig.scanPaths = inputs;
    std::vector<std::string> foundFiles, excludedFiles;
    std::set<std::string>    foundExtentions;
    //umba::scanners::scanFolders( appConfig, argsParser.quet ? umbaLogStreamNul : umbaLogStreamMsg, foundFiles, excludedFiles, foundExtentions);
    umba::scanners::scanFolders( appConfig, umbaLogStreamNul, foundFiles, excludedFiles, foundExtentions);

    // umba::cli_tool_helpers::IoFileType outputFileType = umba::cli_tool_helpers::IoFileType::nameEmpty;

    umba::cli_tool_helpers::IoFileType outputFileType = umba::cli_tool_helpers::IoFileType::nameEmpty;
    if (outputFilename.empty())
    {
        outputFilename = "STDOUT";
    }

    outputFileType = umba::cli_tool_helpers::detectFilenameType(outputFilename, false /* !bInput */);

    #if defined(WIN32) || defined(_WIN32)
    if (outputFileType==umba::cli_tool_helpers::IoFileType::clipboard)
    {
        LOG_ERR_OPT << "invalid output file name"
                    << "\n";
        return 1;
    }
    #endif

    unsigned errCount = 0;


    marty_tr::tr_set_lang_tag_format(langTagFormat);

    std::string curFile; // = fileName;
    unsigned lineNo = 0;

    auto errReportHandler = marty_tr::makeErrReportHandler( marty_tr::DefaultMessageNotFoundHandler(), 
                          [&](const std::string& msgId, const std::string& msgCur, const std::string& msgNew, const std::string& catId, const std::string& langId)
                          {
                              LOG_ERR << umba::formatMessage("translation already exist, language: $(lang), category: $(category), message: '$(msg)'\n"
                                                             "Current text: '$(textCur)'\n"
                                                             "New text    : '$(textNew)'\n"
                                                            )
                                                            .arg("lang"    , langId)
                                                            .arg("category", catId)
                                                            .arg("msg"     , msgId)
                                                            .arg("textCur" , msgCur)
                                                            .arg("textNew" , msgNew)
                                                            .toString()
                                      ; // << "\n";

                              // need warning as error
                              // errCount++;

                              return true; // allow overwite prev translation
                          }
                          ,
                          [&](const std::string& catId, const std::string& msgId) // messageNotFullyTranslated
                          {
                              LOG_WARN_OPT("tr") << umba::formatMessage("message '$(catId):$(msgId)' not fully translated")
                                                                       .arg("catId", catId)
                                                                       .arg("msgId", msgId)
                                                                       .toString()
                                                 << "\n";

                              // need warning as error
                              // errCount++;
                          }
                          ,
                          [&](const std::string& lang, const std::string& langTag) // messageMissingTranslation
                          {
                              if (lang!=langTag)
                              {
                                  LOG_WARN_OPT("tr") << "  " << umba::formatMessage("missing translation: $(langOrg) - $(langTag)")
                                                                           .arg("langOrg", lang)
                                                                           .arg("langTag", langTag)
                                                                           .toString()
                                                 << "\n";
                              }
                              else
                              {
                                  LOG_WARN_OPT("tr") << "  " << umba::formatMessage("missing translation: $(langOrg)")
                                                                           .arg("langOrg",lang)
                                                                           .toString()
                                                 << "\n";
                              }

                              // need warning as error
                              // errCount++;
                          }

                );

    // virtual void messageNotFullyTranslated(const std::string& msgId, const std::string& catId) = 0;
    //  
    // virtual void messageMissingTranslation(const std::string& lang, const std::string& lanfTag) = 0;


    marty_tr::tr_set_err_handler(&errReportHandler);

    encoding::EncodingsApi* pEncodingsApi = encoding::getEncodingsApi();

    for(const auto &fileName : foundFiles)
    {
        if (!argsParser.quet)
        {
            LOG_MSG_OPT << umba::formatMessage("Processing '$(fileName)'")
                                              .arg("fileName",fileName)
                                              .toString()
                        << "\n";
        }

        std::vector<char> fileData;
        if (!umba::filesys::readFile(fileName, fileData))
        {
            LOG_ERR_OPT << umba::formatMessage("failed to read file: '$(fileName)'")
                                              .arg("fileName",fileName)
                                              .toString()
                        << "\n";
            errCount++;
        }

        curFile = fileName;

        std::string relFileName = umba::filename::makeRelPath( appConfig.scanPaths, fileName );
        if (!argsParser.quet)
        {
            // LOG_MSG_OPT << "Relative name   : '" << relFileName << "'" << "\n";
        }

        std::string fileCatId = umba::filename::getPathFile(relFileName);
        if (marty_tr::getLocaleInfo(umba::filename::getName(fileCatId), true /* neutralAllowed */))
        {
            fileCatId = umba::filename::getPath(fileCatId); // Локаль была найдена как последний компонент пути (имя файла) - обрезаем
        }
        else
        {
            // Локаль в имени файла, отделена символом подчеркивания
            std::string::size_type pos = fileCatId.rfind('_');
            if (pos!=fileCatId.npos && marty_tr::getLocaleInfo(umba::filename::getName(std::string(fileCatId, pos+1, std::string::npos)), true /* neutralAllowed */))
            {
                fileCatId = std::string(fileCatId, 0, pos);
            }
        }
        
        if (!argsParser.quet)
        {
            // LOG_MSG_OPT << "File category ID: '" << fileCatId << "'" << "\n";
        }

        

        // fileCatId


        size_t bomSize = 0;
        //const charDataPtr = 
        std::string detectRes = pEncodingsApi->detect( fileData.data(), fileData.size(), bomSize );

        // if (bomSize)
        //     fileText.erase(0,bomSize);

        auto cpId = pEncodingsApi->getCodePageByName(detectRes);
        
        std::string utfText = pEncodingsApi->convert( fileData.data()+bomSize, fileData.size()-bomSize, cpId, encoding::EncodingsApi::cpid_UTF8 );

        marty_tr::all_translations_map_t trMap;
        try
        {
            trMap = marty_tr::tr_parse_translations_data(utfText);
        }
        catch(const std::exception &e)
        {
            //LOG_ERR << ": failed to parse translations file: " << e.what() << "\n";
            LOG_ERR << umba::formatMessage("failed to parse translations file: '$(what)'")
                                          .arg("what",e.what())
                                          .toString()
                    << "\n";
            errCount++;
        }

        if (!marty_tr::tr_replace_category(trMap, substCategoryName, fileCatId))
        {
            LOG_ERR << umba::formatMessage("failed to rename category '$(categoryFrom)' to '$(categoryTo)' - category '$(categoryTo)' already exist")
                                          .arg("categoryFrom",substCategoryName.empty() ? std::string("<EMPTY>") : substCategoryName)
                                          .arg("categoryTo",fileCatId)
                                          .toString()
                    << "\n";
        }


        marty_tr::tr_add_custom_translations(trMap);

    } // for(const auto &fileName : foundFiles)


    //auto allTrMap = marty_tr::tr_get_all_translations();

    // Проверяем переводы


    // typedef StringStringMap<std::string>                                    translations_map_t;
    // typedef std::unordered_map<std::string,translations_map_t>              category_translations_map_t;
    // typedef std::unordered_map<std::string,category_translations_map_t>     all_translations_map_t;

    #if 0
    std::set<std::string> foundLangs;

    std::map<std::string, std::set<std::string> >                      msgLangs;


    // Затем пробегаемся по всем языкам, для каждого языка пробегаемся по категориям и сообщениям.

    for(const auto &langKvp : allTrMap)
    {
        const auto &langId  = langKvp.first;
        const auto &catMap  = langKvp.second;

        foundLangs.insert(langId);

        for(const auto &catKvp : catMap)
        {
            const auto &catId  = catKvp.first;
            const auto &msgMap = catKvp.second;

            for(const auto &msgKvp : msgMap)
            {
                const auto &msgId   = msgKvp.first;
                const auto &msgText = msgKvp.second;

                // Склеиваем категорию:сообщение в ключ, и инкрементируем элемент map по этому ключу.

                std::string msgFullId = catId + std::string(":") + msgId;

                msgLangs[msgFullId].insert(langId); // add found translation
            }

        } // cat

    } // lang

    for(const auto &msgCatKvp : msgLangs)
    {
        const auto &msgCat   = msgCatKvp.first;
        const auto &msgLangs = msgCatKvp.second;

        if (msgLangs.size()!=foundLangs.size())
        {
            LOG_WARN_OPT("tr") << umba::formatMessage("message '$(msgCat)' not fully translated")
                                                     .arg("msgCat",msgCat)
                                                     .toString()
                               << "\n";
            // errCount++;

            // iterate through all found langs
            for(const auto &lang : foundLangs)
            {
                std::set<std::string>::const_iterator it = msgLangs.find(lang);
                if (it!=msgLangs.end()) // если язык не найден для данного сообщения, то выводим сообщение
                {
                    auto langTag = marty_tr::formatLangTag(lang, marty_tr::ELangTagFormat::langTag);
                    LOG_WARN_OPT("tr") << "  " << umba::formatMessage("missing translation: $(langOrg) - $(langTag)")
                                                             .arg("langOrg",lang)
                                                             .arg("langTag" ,langTag)
                                                             .toString()
                                       << "\n";
                }

            } // for(const auto &lang : foundLangs)
        
        } // if (msgLangs.size()!=foundLangs.size())

    } // for(const auto &msgCatKvp : msgLangs)
    #endif

    marty_tr::tr_check_translation_completeness();

    // if (!argsParser.quet)
    // {
    //     LOG_MSG_OPT << umba::formatMessage("Processing '$(fileName)'")
    //                                       .arg("fileName",fileName)
    //                                       .toString()
    //                 << "\n";
    // }

    if (!errCount || bForce)
    {
        
        std::string finalTrText = marty_tr::tr_serialize_translations(marty_tr::tr_get_all_translations(), 2 /* indent */);

        if (outputFileType!=umba::cli_tool_helpers::IoFileType::stdoutFile)
        {
            finalTrText = marty_cpp::converLfToOutputFormat(finalTrText, outputLinefeed);
        }
        
        if (!umba::cli_tool_helpers::writeFile(outputFileType, outputFilename, finalTrText, bOverwrite))
        {
            LOG_ERR_OPT << umba::formatMessage("failed to write output file: '$(fileName)'")
                                              .arg("fileName",outputFilename)
                                              .toString()
                        << "\n";
    
            return 1;
        }
    }
    else
    {
        LOG_ERR << umba::formatMessage("some errors occur, output not written")
                                      .toString()
                << "\n";
    }

    //TODO: !!! если файл существует, его надо обнулить


    return 0;
}

