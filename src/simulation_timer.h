#pragma once

#include <vector>
#include <functional>
#include <map>
#include <iostream>

#include  "constants.h"

class Job {
public:
    Job(std::uint32_t id, const std::function<void()> & job)
    : id_(id)
    , job_(job)
    {}

    void do_job()
    {
        job_();
    }

    std::uint32_t id()
    {
        return id_;
    }

private:
    std::uint32_t id_;
    std::function<void()> job_;
};

class SimulationTimer {
public:
    SimulationTimer();
    inline float time() const
    {
        return time_;
    }

    inline std::uint32_t register_job(float start_time, const std::function<void()> & callback) const
    {
        if (constants::DEBUG_ENABLED) {
            std::cout << "SimulationTimer::" << __func__
                      << " registered job with start time: " << start_time
                      << " and id: " << last_job_id_
                      << " at time: " << time_ << std::endl;
        }
        jobs_.insert({start_time, Job(last_job_id_, callback)});
        return last_job_id_++;
    }

    inline float remove_job(std::uint32_t id) const
    {
        for (auto jobs_iterator = jobs_.begin(); jobs_iterator != jobs_.end(); ++jobs_iterator) {
            if (jobs_iterator->second.id() == id) {
                float old_departure_time = jobs_iterator->first;
                jobs_.erase(jobs_iterator);
                if (constants::DEBUG_ENABLED) {
                    std::cout << "SimulationTimer::" << __func__
                            << " erased job: " << id << std::endl;
                }
                return old_departure_time;
            }
        }
        throw std::runtime_error("remove_job called with invalid id");
    }

    void advance_time();

private:
    float time_;
    mutable std::uint32_t last_job_id_;
    mutable std::multimap<float, Job> jobs_;
};
