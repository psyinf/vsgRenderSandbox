#include <kafka/KafkaConsumer.h>
using namespace kafka;
using namespace kafka::clients;
using namespace kafka::clients::consumer;


/**
 * Update stream via Kafka, collecting consumerRecords usually containing json representations of the entites
 
 */
const Properties props({
    {"bootstrap.servers", {"127.0.0.1:9092"}},
    {"enable.idempotence", {"true"}},
});
class UpdateStream
{
public:
    const std::string topic = "test";

    KafkaConsumer consumer = KafkaConsumer(props);
    bool          debug    = true;

    std::vector<ConsumerRecord> back_buffer;

    UpdateStream()
    {
        // Subscribe to topics
        consumer.subscribe({topic});
    }
    void swap(std::vector<ConsumerRecord>& target)
    {
        std::swap(target, back_buffer);
    }


    void run()
    {
        while (true)
        {
            auto records = consumer.poll(std::chrono::milliseconds(10));
            for (const auto& record : records)
            {
                // TODO: use dedicated message type here
                // In this example, quit on empty message
                if (record.value().size() == 0)
                {
                    std::cerr << "Quit due to empty message" << std::endl;
                    return;
                }


                if (!record.error())
                {
                    if (debug)
                    {
                        std::cout << "% Got a new message..." << std::endl;
                        std::cout << "    Topic    : " << record.topic() << std::endl;
                        std::cout << "    Partition: " << record.partition() << std::endl;
                        std::cout << "    Offset   : " << record.offset() << std::endl;
                        std::cout << "    Timestamp: " << record.timestamp().toString() << std::endl;
                        std::cout << "    Headers  : " << toString(record.headers()) << std::endl;
                        std::cout << "    Key   [" << record.key().toString() << "]" << std::endl;
                        std::cout << "    Value [" << record.value().toString() << "]" << std::endl;
                    }
                    back_buffer.emplace_back(record);
                }
                else if (record.error())
                {
                    std::cerr << record.toString() << std::endl;
                }
            }
        }
    }
};