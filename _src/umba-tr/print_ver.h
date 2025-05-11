#pragma once

// https://stackoverflow.com/questions/11697820/how-to-use-date-and-time-predefined-macros-in-as-two-integers-then-stri
void printOnlyVersion()
{
    std::cout<<appVersion<<"\n";
}

void printNameVersion( const std::string &indent = "" )
{
    std::cout<<indent << appFullName << " version ";
    printOnlyVersion();
    //<<rdlcVersion<<"\n";
}

void printCommitHash( const std::string &indent = "" )
{
    if (appCommitHash.empty())
        return;
    std::cout<<indent<<"#"<<appCommitHash<<"\n";
}

void printBuildDateTime( const std::string &indent = "" )
{
    std::cout<<indent<<"Built at "<<appBuildDate<<" "<<appBuildTime<<"\n";
}


