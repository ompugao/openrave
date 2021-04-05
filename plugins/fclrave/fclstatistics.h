// -*- coding: utf-8 -*-
#ifndef OPENRAVE_FCL_STATISTICS
#define OPENRAVE_FCL_STATISTICS


#ifdef FCLUSESTATISTICS

#include "plugindefs.h"
#include <sstream>
#include <fstream>


namespace fclrave {

class FCLStatistics;

static std::vector<boost::weak_ptr<FCLStatistics> > globalStatistics;
static EnvironmentMutex log_out_mutex;
typedef std::chrono::time_point<std::chrono::high_resolution_clock> time_point;
typedef std::chrono::duration<double> duration;

class FCLStatistics {
public:
    FCLStatistics(std::string const& key, int id) {
        name = str(boost::format("%d;%s")%id%key);
        RAVELOG_WARN_FORMAT("FCL STATISTICS %s", name);
        currentTimings.reserve(64); // so that we don't allocate during the timing
    }

    ~FCLStatistics() {
#ifndef FCL_STATISTICS_DISPLAY_CONTINUOUSLY
        Display();
#endif
    }

    void DisplayAll() {
        FOREACH(itstat, globalStatistics) {
            boost::shared_ptr<FCLStatistics> pstat = itstat->lock();
            if( pstat ) {
                pstat->Display();
            }
        }
    }

    void DisplaySingle(const std::string& label, std::vector<time_point> timings) {
        std::stringstream ss;
        std::vector<time_point>::iterator it = timings.begin();
        time_point t = *it;
        while(++it != timings.end()) {
            ss << ";" << (*it - t).count();
            t = *it;
        }
        RAVELOG_WARN_FORMAT("FCL STATISTICS;%s%s", label % ss.str());
    }

    void Display(const std::string& fileprefix="") {
        EnvironmentMutex::scoped_lock lock(log_out_mutex);
        std::fstream f(fileprefix+"fclstatistics.log", std::fstream::out | std::fstream::app);
        FOREACH(ittiming, timings) {
            f << ittiming->first;
            size_t maxTimingCount = 0;
            FOREACH(ittimingvector, ittiming->second) {
                const std::vector<time_point>& timingvector = ittimingvector->first;
                const bool bincollision = ittimingvector->second;
                f << ";" << bincollision << "!";
                maxTimingCount = std::max(maxTimingCount, timingvector.size() - 1);
                std::vector<time_point>::const_iterator it = timingvector.begin();
                time_point t = *it;
                while(++it != timingvector.end()) {
                    f << "|" << std::chrono::duration_cast<std::chrono::nanoseconds>(*it - t).count();
                    //t = *it;
                }
            }
            f << std::endl;
            //f << ";" << maxTimingCount << std::endl;
        }
    }

    void StartManualTiming(std::string const& label) {
        currentTimingLabel = str(boost::format("%s;%s")%name%label);
        currentTimings.resize(0);
        currentTimings.push_back(std::chrono::high_resolution_clock::now());
    }

    void StopManualTiming(bool bincollision=false) {
        currentTimings.push_back(std::chrono::high_resolution_clock::now());
        timings[currentTimingLabel].push_back(std::make_pair(currentTimings, bincollision));
#ifdef FCL_STATISTICS_DISPLAY_CONTINUOUSLY
        DisplaySingle(currentTimingLabel, currentTimings);
#endif
    }

    void Reset() {
        currentTimings.clear();
        timings.clear();
    }

    void AddTimepoint() {
        currentTimings.push_back(std::chrono::high_resolution_clock::now());
    }

    struct Timing {
        Timing(FCLStatistics& statistics) : _statistics(statistics) {
        }
        ~Timing() {
            _statistics.StopManualTiming();
        }
private:
        FCLStatistics& _statistics;
    };

    Timing StartTiming(std::string const& label) {
        StartManualTiming(label);
        return Timing(*this);
    }

    std::string name;
    std::string currentTimingLabel;
    std::vector<time_point> currentTimings;
    std::map< std::string, std::vector< std::pair<std::vector<time_point>, bool> > > timings;
};

typedef boost::shared_ptr<FCLStatistics> FCLStatisticsPtr;

#define SETUP_STATISTICS(statistics, userdatakey, id) \
    statistics = boost::make_shared<FCLStatistics>(userdatakey, id); \
    globalStatistics.push_back(boost::weak_ptr<FCLStatistics>(statistics))

#define START_TIMING(statistics, label) statistics->StartManualTiming(label)
#define STOP_TIMING(statistics, bincollision) statistics->StopManualTiming(bincollision)

#define ADD_TIMING(statistics) statistics->AddTimepoint()

#define DISPLAY(statistics, fileprefix) statistics->Display(fileprefix)

#define DISPLAY_ALL(statistics) statistics->DisplayAll()

#define RESET_TIMINGS(statistics) statistics->Reset()

} // fclrave

#else // FCLUSESTATISTICS is not defined

namespace fclrave {

class FCLStatistics {
};

#define SETUP_STATISTICS(statistics, userdatakey, id) do {} while(false)
#define START_TIMING(statistics, label) do {} while(false)
#define STOP_TIMING(statistics, bincollision) do {} while(false)
#define ADD_TIMING(statistics) do {} while(false)
#define DISPLAY(statistics, fileprefix) do {} while(false)
#define DISPLAY_ALL(statistics) do {} while(false)
#define RESET_TIMINGS(statistics) do {} while(false)

}
#endif



#endif
