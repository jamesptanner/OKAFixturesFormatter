// fixtures loader.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <regex>
#include <map>
#include <Windows.h>

std::string readFile(std::wstring filePath)
{
    std::ifstream file(filePath);
    return std::string((std::istreambuf_iterator<char>(file) ),
        (std::istreambuf_iterator<char>() ) );
}

std::map<std::string,std::string> readMappingFile(std::wstring filePath, const bool & reverse = false)
{
    std::string content = readFile(filePath);
    char* lineToken = std::strtok((char*)content.c_str(),"\n");    
    int currentColumns;
    std::map<std::string,std::string> ret;

    while(lineToken != NULL)
    {
        std::string line(lineToken);
        std::string key;
        std::string value;

        int splitchar = line.find_first_of(":");
        key = line.substr(0,splitchar);
        value = line.substr(splitchar+1);
        if(reverse)
        {
            ret[value] = key;
        }
        else
        {
            ret[key] = value;
        }
        lineToken = std::strtok(NULL,"\n");
    }
    return ret;
}

std::vector<std::string> readFilter(std::wstring filePath)
{
    std::string content = readFile(filePath);
    char* lineToken = std::strtok((char*)content.c_str(),"\n");    
    int currentColumns;
    std::vector<std::string> ret;

    while(lineToken != NULL)
    {
        ret.push_back(lineToken);
        lineToken = std::strtok(NULL,"\n");
    }
    return ret;
}
struct Fixture
{
    std::string venue;
    std::string date;
    std::string time;
    std::string homeTeam;
    std::string awayTeam;
};
typedef std::vector<Fixture> FixtureList;
std::string dateCaptureRegex("([0-9]+/[0-9]+/[0-9]+)");
std::regex dateRegex(dateCaptureRegex);

bool isDateLine(const std::string & candidate)
{
    return std::regex_search(candidate,dateRegex);
}

bool isFixturesLine(const std::string & candidate)
{
    return candidate.find(",vs,") != std::string::npos;
}
bool isHeaderLine(const std::string & candidate)
{
    return candidate.find("Team 1") != std::string::npos;
}
bool isUpdatedLocation(const std::string & candidate)
{
    return std::regex_search(candidate,std::regex("[^,],,,,,,"));
}
void processHeaderLine(const std::string & headerLine,FixtureList & fixtureTemplates)
{
    size_t cells = std::count(headerLine.begin(), headerLine.end(), ',');
    //std::cout << cells << " cells on row." << std::endl;

    //split line into blocks of 7 cells (or a fixture)
    std::string testLine = headerLine;
    std::vector<std::string> fixtures;
    int pos = 0;
    while (pos != std::string::npos)
    { 
        pos=0;
        int fixtureStart = 0;
        for(int i = 0; i < 7 && pos != std::string::npos; i++)
        {
            pos = testLine.find(",",pos+1);
        }
        std::string fixture = testLine.substr(fixtureStart,pos-fixtureStart);
        if(pos!= std::string::npos)
        {
            testLine = testLine.substr(pos+1);
        }
        fixtures.push_back(fixture);

    }
    int i = 0;
    //for(Fixture fixtureTemplate : fixtureTemplates)
    FixtureList::iterator start = fixtureTemplates.begin();
    FixtureList::iterator end  = fixtureTemplates.end();
    for(auto iter= start; iter !=end;++iter)
    {
        iter->venue = std::string(fixtures[i].substr(0,fixtures[i].find_first_of(',')));
        i++;
    }
}
int strpos(const char *haystack, char *needle, int nth)
{
    char *res = const_cast<char*>(haystack);
    for(int i = 1; i <= nth; i++)
    {
        res = strstr(res, needle);
        if (!res)
            return -1;
        else if(i != nth)
            res = res++;
    }
    return res - haystack;
}
std::vector<Fixture> processFixtureLine(const std::string & fixtureLine,FixtureList & fixtureTemplate)
{
    size_t cells = std::count(fixtureLine.begin(), fixtureLine.end(), ',');
    std::string testLine = fixtureLine;
    //split line into blocks of 7 cells (or a fixture)
    std::vector<std::string> fixtures;
    int pos = 0;
    while (pos != std::string::npos)
    { 
        pos=0;
        int fixtureStart = 0;

        pos = strpos(testLine.c_str(),",",7);

        std::string fixture = testLine.substr(fixtureStart,pos-fixtureStart);
        if(pos!= std::string::npos)
        {
            testLine = testLine.substr(pos+1);
        }
        fixtures.push_back(fixture);
    }

    FixtureList returnedFixtures;
    int i = 0;
    for(std::string fixture : fixtures)
    {
        if(isFixturesLine(fixture))
        {
            //split the fixture line into its components.
            Fixture currentFixture = fixtureTemplate[i];
            int pos = -1;
            currentFixture.time = fixture.substr(0,pos = fixture.find(",",pos +1));
            int teamAStart = pos+1;
            currentFixture.homeTeam = fixture.substr(teamAStart,(pos = fixture.find(",",teamAStart))-teamAStart);
            int vsStart = pos+1;
            std::string vs = fixture.substr(vsStart,(pos = fixture.find(",",vsStart))-vsStart);
            int teamBStart = pos+1;
            currentFixture.awayTeam = fixture.substr(teamBStart,(pos = fixture.find(",",teamBStart))-teamBStart);
            returnedFixtures.push_back(currentFixture);
        }
        else
        {
            if(isUpdatedLocation(fixture))
            {
                //__debugbreak();
                fixtureTemplate[i].venue = std::string(fixtures[i].substr(0,fixtures[i].find_first_of(',')));
            }
            else if(fixture.find_first_not_of(",")!= std::string::npos)
            {
                std::cout << "Unknown fixture:" << fixture << std::endl << "    of line:" << fixtureLine << std::endl;
                //__debugbreak();
            }
        }
        i++;
    }
    return returnedFixtures;
}


std::ostream& operator<<(std::ostream& os, const Fixture& request)
{
std::stringstream str;
        str << "{\"date\":\"" << request.date << "\","
        << "\"time\":\"" << request.time << "\","
        << "\"home\":\"" << request.homeTeam << "\","
        << "\"away\":\"" << request.awayTeam << "\","
        << "\"venue\":\"" << request.venue << "\"}";
    return os << str.str().c_str(); 
};

std::vector<Fixture> processContent(const std::string content)
{
    char* lineToken = std::strtok((char*)content.c_str(),"\n");

    int currentColumns;
    FixtureList fixturesTemplates;
    FixtureList fixturesList;
    while(lineToken!= NULL)
    {
        std::string currentLine(lineToken);
        if(isDateLine(currentLine))
        {
            fixturesTemplates.clear();
            auto firstDate = std::sregex_iterator(currentLine.begin(),currentLine.end(),dateRegex);
            auto lastDate = std::sregex_iterator();
            currentColumns = std::distance(firstDate,lastDate);
            //std::cout << "Found " << currentColumns << " dates" << std::endl;

            for(std::sregex_iterator i = firstDate; i != lastDate; ++i)
            {
                Fixture templateFixture;
                templateFixture.date = i->str();
                fixturesTemplates.push_back(templateFixture);
            }

        }
        else if(isFixturesLine(currentLine))
        {
            std::regex fixtureregex(",vs,");
            auto firstFixture = std::sregex_iterator(currentLine.begin(),currentLine.end(),fixtureregex);
            auto lastFixture = std::sregex_iterator();
            int fixtureCount = std::distance(firstFixture,lastFixture);

            FixtureList temp = processFixtureLine(currentLine, fixturesTemplates);
            fixturesList.insert(fixturesList.end(),temp.begin(), temp.end());
        }
        else if(isHeaderLine(currentLine))
        {
            processHeaderLine(currentLine,fixturesTemplates);
        }
        lineToken = std::strtok(NULL,"\n");
    }
    std::cout << "processed Fixtures:" << std::endl;
    for(Fixture fix : fixturesList)
    {
        std::stringstream debug;
        debug << "     " << fix << "," << std::endl;
        OutputDebugStringA(debug.str().c_str());
//        std::cout << "     " << fix << "," << std::endl;
    }
    std::cout << "total fixtures : " << fixturesList.size() << std::endl;
    return fixturesList;
}

bool noFilter = false;
bool overrideVenue = true;
bool overrideTime = true; 
void formatFixtures(const FixtureList & fixtures, const std::map<std::string,std::string> & venues, const std::map<std::string,std::string> & teams, const std::vector<std::string> filter)
{
    FixtureList skippedfixtures;
    for(Fixture fixture : fixtures)
    {
        if(std::find(filter.begin(),filter.end(),fixture.homeTeam) != filter.end() && std::find(filter.begin(),filter.end(),fixture.homeTeam) != filter.end() || noFilter)
        {
            if(teams.find(fixture.homeTeam)!= teams.end() && teams.find(fixture.awayTeam)!= teams.end() &&
                 ((venues.find(fixture.venue) != venues.end()) || overrideVenue) && 
                 ((fixture.time != "tbd") || overrideTime))
            {
                std::cout << teams.find(fixture.homeTeam)->second << "," << teams.find(fixture.awayTeam)->second << "," << 
                fixture.date << "," << (fixture.time != "tbd" ? fixture.time : "00:00") <<
                "," << ((venues.find(fixture.venue) != venues.end()) ? venues.find(fixture.venue)->second : "4") << std::endl;
            }
            else
            {
                skippedfixtures.push_back(fixture);
            }
        }
        else if((std::find(filter.begin(),filter.end(),fixture.homeTeam)!= filter.end()) != (std::find(filter.begin(),filter.end(),fixture.homeTeam) != filter.end()))
        {
            skippedfixtures.push_back(fixture);
        }
    }
    if(skippedfixtures.size() > 0)
    {
        std::cout << "had " << skippedfixtures.size() << " unprocessable fixtures." << std::endl;
        int time=0;
        int team=0;
        int venue=0;

        for(Fixture fixture : skippedfixtures)
        {
            std::cout << std::setw(90) << std::left << fixture << "reason:" ;

            if(fixture.time == "tbd")
            {
                std::cout << "no start time. ";
                time++;
            }
            if(teams.find(fixture.homeTeam) == teams.end())
            {
                std::cout << "Home Team not registed with FL. ";
                team++;
            }
            if(teams.find(fixture.awayTeam) == teams.end())
            {
                std::cout << "Away Team not registed with FL. ";
                team++;
            }
            if(venues.find(fixture.venue) == venues.end())
            {
                std::cout << "Couldn't identify the venue. ";
                venue++;
            }
            std::cout << std::endl;
        }
        std::cout << "Reason breakdown time: " << time << " team registration: " << team << " unknown venue: " << venue << std::endl; 
    }
    
}
int _tmain(int argc, _TCHAR* argv[])
{
    std::string fileContent = readFile(argv[1]);
    std::map<std::string,std::string> venues = readMappingFile(argv[2]);
    std::map<std::string,std::string> teams = readMappingFile(argv[3],true);
    std::vector<std::string> teamFilter = readFilter(argv[4]);
    FixtureList fixtures = processContent(fileContent);
    formatFixtures(fixtures,venues,teams,teamFilter);
    return 0;
}

