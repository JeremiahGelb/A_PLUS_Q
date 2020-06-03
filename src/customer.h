#pragma once

#include <memory>
#include <string>
#include <sstream>
#include <functional>

class Customer {
public:
    Customer(std::uint32_t id,
             float arrival_time,
             float service_time,
             bool serviced,
             float departure_time)
    : id_(id)
    , arrival_time_(arrival_time)
    , service_time_(service_time)
    , serviced_(serviced)
    , departure_time_(departure_time)
    {}

    Customer(std::string str)
    {
        char comma;
        std::uint32_t id;
        float arrival_time;
        float service_time;
        bool serviced;
        float departure_time;

        std::stringstream ss(str);
        ss >> id;
        ss >> comma;
        ss >> arrival_time;
        ss >> comma;
        ss >> service_time;
        ss >> comma;
        ss >> serviced;
        ss >> comma;
        ss >> departure_time;

        id_ = id;
        arrival_time_ = arrival_time;
        service_time_ = service_time;
        serviced_ = serviced;
        departure_time_ = departure_time;
    }

    std::string to_string()
    {
        std::stringstream ss;
        ss << id_
           << ','
           << arrival_time_
           << ','
           << service_time_
           << ','
           << serviced_
           << ','
           << departure_time_
           << '\n';

        return ss.str();
    }

    std::uint32_t id() const
    {
        return id_;
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
    float departure_time() const
    {
        return departure_time_;
    }

    void set_serviced(bool serviced) {
        serviced_ = serviced;
    }

    void set_departure_time(float departure_time) {
        departure_time_ = departure_time;
    }

private:
    std::uint32_t id_; // unique id for debug purposes
    float arrival_time_; // time they enter system
    float service_time_; // time their job will take
    bool serviced_; // was request fulfilled
    float departure_time_;
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
                                               float service_time)
{
    return std::make_shared<Customer>(id, arrrival_time, service_time, false, 0);
}

inline std::shared_ptr<Customer> make_customer(std::string str)
{
    return std::make_shared<Customer>(str);
}

using CustomerRequest = std::function<void(std::shared_ptr<Customer>)>;
using CustomerRequestHandler = std::function<void(const CustomerRequest &)>;