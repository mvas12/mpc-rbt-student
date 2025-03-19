#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/float32.hpp"

using namespace std::chrono_literals;

/* This example creates a subclass of Node and uses std::bind() to register a
* member function as a callback from the timer. */

class MinimalPublisher : public rclcpp::Node
{
  public:
    MinimalPublisher()
    : Node("M240466_publisher"), count_(0)
    {
      // Deklarace parametrů pro minimální a maximální napětí
      this->declare_parameter<float>("min_voltage", 36.0);  // výchozí minimální napětí (36V)
      this->declare_parameter<float>("max_voltage", 42.0);  // výchozí maximální napětí (42V)
      publisher_ = this->create_publisher<std_msgs::msg::String>("M240466_publisher", 10);
      battery_percentage_publisher_ = this->create_publisher<std_msgs::msg::Float32>("battery_percentage", 10);
      battery_voltage_subscriber_ = this->create_subscription<std_msgs::msg::Float32>("battery_voltage", 10, std::bind(&MinimalPublisher::battery_voltage_callback, this, std::placeholders::_1));
      timer_ = this->create_wall_timer(std::chrono::seconds(1),
            std::bind(&MinimalPublisher::timer_callback, this));
    }

private:
    void timer_callback()
    {
      auto message = std_msgs::msg::String();
      message.data = "M240466_publisher";
      RCLCPP_INFO(this->get_logger(), "'%s'", message.data.c_str());
      publisher_->publish(message);
      count_++;
        if (count_ >= 10) {
            RCLCPP_INFO(this->get_logger(), "Published 10 messages, shutting down.");
            rclcpp::shutdown();
    }
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
    size_t count_;
};


    void battery_voltage_callback(const std_msgs::msg::Float32::SharedPtr msg)
    {
        // Přepočet napětí na procenta
        float voltage = msg->data;
        float percentage = 0.0;

        if (voltage >= min_voltage_ && voltage <= max_voltage_) {
            percentage = (voltage - min_voltage_) / (max_voltage_ - min_voltage_) * 100.0;
        }

        // Vytvoření a publikování zprávy o procentu baterie
        auto percentage_msg = std_msgs::msg::Float32();
        percentage_msg.data = percentage;
        battery_percentage_publisher_->publish(percentage_msg);

        RCLCPP_INFO(this->get_logger(), "Battery Voltage: %.2fV, Battery Percentage: %.2f%%", voltage, percentage);
    }

    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
    rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr battery_percentage_publisher_;
    rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr battery_voltage_subscriber_;
    rclcpp::TimerBase::SharedPtr timer_;
    size_t count_;
    
    // Parametry pro minimální a maximální napětí
    float min_voltage_;
    float max_voltage_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MinimalPublisher>());
  rclcpp::shutdown();
  return 0;
}
