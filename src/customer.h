#pragma once

#include <memory>
#include <string>
#include <sstream>
#include <functional>
#include <utility>
#include <algorithm>

enum class CustomerEventType {
    ENTERED,
    EXITED,
    DROPPED_BY
};

enum class PlaceType {
    QUEUE,
    SERVER
};

struct CustomerEvent {
    CustomerEvent(const CustomerEventType event_type,
                  const PlaceType place_type,
                  const std::string & place_name,
                  const float time)
    : event_type_(event_type)
    , place_type_(place_type)
    , place_name_(place_name)
    , time_(time)
    {}

    const CustomerEventType event_type_;
    const PlaceType place_type_;
    const std::string & place_name_;
    const float time_;
};

namespace {

std::string to_string(CustomerEventType type)
{
    switch (type) {
    case CustomerEventType::ENTERED:
        return "ENTERED";
    case CustomerEventType::EXITED:
        return "EXITED";
    case CustomerEventType::DROPPED_BY:
        return "DROPPED_BY";
    }
}

std::string to_string(PlaceType type)
{
    switch (type) {
    case PlaceType::QUEUE:
        return "QUEUE";
    case PlaceType::SERVER:
        return "SERVER";
    }
}

std::string to_string(const CustomerEvent & event) {
    std::stringstream ss;
    ss << to_string(event.event_type_)
       << " " << to_string(event.place_type_)
       << " " << event.place_name_
       << " at " << event.time_;
    return ss.str();
}

} // anonymous

class Customer {
public:
    Customer(std::uint32_t id,
             float arrival_time,
             std::uint32_t priority,
             float service_time,
             bool serviced,
             float departure_time)
    : id_(id)
    , arrival_time_(arrival_time)
    , priority_(priority)
    , service_time_(service_time)
    , serviced_(serviced)
    , departure_time_(departure_time)
    {}

    std::string to_string(bool verbose = false)
    {
        std::stringstream ss;
        ss << "id: "
           << id_
           << " Arrival Time: "
           << arrival_time_
           << " Priority: "
           << priority_
           << " Service Time: "
           << service_time_
           << " Serviced: "
           << serviced_
           << " Departure Time: "
           << departure_time_;

        if (verbose) {
            for (const auto & event : events_) {
                ss << std::endl << ::to_string(event);
            }
        }

        return ss.str();
    }

    std::uint32_t id() const
    {
        return id_;
    }

    std::uint32_t priority() const
    {
        return priority_;
    }

    float arrival_time() const
    {
        return arrival_time_;
    }

    float service_time() const
    {
        return service_time_;
    }

    bool serviced() const
    {
        return serviced_;
    }

    float system_time() const
    {
        return departure_time_ - arrival_time_;
    }

    float total_waiting_time() const
    {
        float waiting_time = 0.0;
        if (!serviced_) {
            throw std::runtime_error("Invalid to calculate waiting_time on unserviced customer");
        }

        if (events_.size() < 2) {
            throw std::runtime_error("Too few events to calculate waiting_time");
        }

        for (std::uint32_t i = 0; i < events_.size() - 1; ++i) {
            const auto & event = events_[i];
            const auto & next_event = events_[i+1];

            if (event.event_type_ == CustomerEventType::ENTERED && event.place_type_ == PlaceType::QUEUE) {
                if (next_event.event_type_ == CustomerEventType::EXITED && next_event.place_name_ == event.place_name_) {
                        waiting_time += (next_event.time_ - event.time_);
                        // ++i // (could do this but it might not actually save time)
                } else {
                    throw std::runtime_error("Event following ENTER wasn't the right EXIT");
                }
            }
        }

        return waiting_time;
    }

    float waiting_time(const std::string & place_name)
    {
        float waiting_time = 0.0;
        if (!serviced_) {
            throw std::runtime_error("Invalid to calculate waiting_time on unserviced customer");
        }
        if (events_.size() < 2) {
            throw std::runtime_error("Too few events to calculate waiting_time");
        }
        for (std::uint32_t i = 0; i < events_.size() - 1; ++i) {
            const auto & event = events_[i];
            const auto & next_event = events_[i+1];

            if (event.event_type_ == CustomerEventType::ENTERED && event.place_name_ == place_name) {
                if (next_event.event_type_ == CustomerEventType::EXITED && next_event.place_name_ == place_name) {
                        waiting_time += (next_event.time_ - event.time_);
                } else {
                    throw std::runtime_error("Event following ENTER wasn't the right EXIT");
                }
            }
        }

        return waiting_time;
    }

    bool entered(const std::string & place_name)
    {
        for (const auto & event : events_) {
            if (event.event_type_ == CustomerEventType::ENTERED
                && event.place_name_ == place_name) {
                return true;
            }
        }
        return false;
    }

    std::uint32_t entrances(const std::string & place_name)
    {
        return std::count_if(events_.begin(),
                             events_.end(),
                             [&place_name](const CustomerEvent & event) {
                                 return event.event_type_ == CustomerEventType::ENTERED
                                        && event.place_name_ == place_name;
                             });
    }

    std::string dropped_by()
    {
        const auto & event = events_.back();
        if (event.event_type_ == CustomerEventType::DROPPED_BY) {
            return event.place_name_;
        } else {
            throw std::runtime_error("dropped_by called on customer who wasn't dropped");
            return "";
        }
    }

    float departure_time() const
    {
        return departure_time_;
    }

    void set_service_time(float service_time)
    {
        service_time_ = service_time;
    }

    void set_serviced(bool serviced)
    {
        serviced_ = serviced;
    }

    void set_departure_time(float departure_time)
    {
        departure_time_ = departure_time;
    }

    void add_event(const CustomerEvent & event) {
        events_.push_back(event);
    }

private:
    std::uint32_t id_; // unique id for debug purposes
    float arrival_time_; // time they enter system
    std::uint32_t priority_;
    float service_time_; // time their next job will take
    bool serviced_; // was request fulfilled
    float departure_time_;
    std::vector<CustomerEvent> events_;
    // could refactor this to just save explicitly what I need rather than save every event and calculate but
    // this is a useful debugging tool.
};

inline bool operator==(const Customer & lhs, const Customer & rhs)
{
    return lhs.id() == rhs.id()
           && lhs.arrival_time() == rhs.arrival_time()
           && lhs.service_time() == rhs.service_time()
           && lhs.serviced() == rhs.serviced()
           && lhs.departure_time() == rhs.departure_time();
}

inline std::shared_ptr<Customer> make_customer(std::uint32_t id,
                                               float arrrival_time,
                                               std::uint32_t priority = 0)
{
    return std::make_shared<Customer>(id, arrrival_time, priority, 0.0, false, 0.0);
}

using CustomerRequest = std::function<void(const std::shared_ptr<Customer> &)>;
using CustomerRequestHandler = std::function<void(const CustomerRequest &)>;
