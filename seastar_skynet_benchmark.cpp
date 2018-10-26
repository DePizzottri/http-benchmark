#include <boost/iterator/counting_iterator.hpp>
#include "core/app-template.hh"
#include "core/sleep.hh"
#include "core/gate.hh"
#include "core/thread.hh"


// seastar::future<> service_loop() {
//     return seastar::do_with(seastar::listen(seastar::make_ipv4_address({1234})),
//             [] (auto& listener) {
//         return seastar::keep_doing([&listener] () {
//             return listener.accept().then(
//                 [] (seastar::connected_socket s, seastar::socket_address a) {
//                     std::cout << "Accepted connection from " << a << "\n";
//                 });
//         });
//     });
// }


// seastar::future<> f() {
//     return seastar::parallel_for_each(boost::irange<unsigned>(0, seastar::smp::count),
//             [] (unsigned c) {
//         return seastar::smp::submit_to(c, [] {
//                 std::cout << seastar::thread::running_in_thread() << std::endl;
//             });
//     });
// }

seastar::future<std::uint64_t> skynet(std::uint64_t num, std::uint64_t size, std::uint64_t div) {
    if ( size != 1) {
        size /= div;

        std::vector<seastar::future<std::uint64_t> > results;
        results.reserve(div);

        for (std::uint64_t i = 0; i != div; ++i) {
            std::uint64_t sub_num = num + i * size;
            results.emplace_back(
                skynet(sub_num, size, div)
            );
        }

        auto p = seastar::make_shared(std::move(results));
        return seastar::when_all(p->begin(), p->end()).then(
            [p] (std::vector<seastar::future<uint64_t>> ret) {
                std::uint64_t sum = 0;
                for (auto& r: ret) {
                    sum += r.get0();            
                }
                return sum;
            }
        );
    }

    return seastar::make_ready_future<std::uint64_t>(num);
}

seastar::future<std::uint64_t> skynet_start(std::uint64_t num, std::uint64_t size, std::uint64_t div) {
    const auto parallelism_factor = seastar::smp::count;

    if (div % parallelism_factor != 0) {
        std::cout << "Parallelism may be unfair!" <<std::endl;
    }

    size /= div;

    std::vector<seastar::future<std::uint64_t> > results;
    results.reserve(parallelism_factor);

    for (std::uint64_t i = 0; i != div; ++i) {
        std::uint64_t sub_num = num + i * size;
        auto shard = parallelism_factor == 1 ? 0 : i % parallelism_factor;
        results.emplace_back(
            seastar::smp::submit_to(shard, [=] {return skynet(sub_num, size, div);})
        );
    }

    auto p = seastar::make_shared(std::move(results));
    return seastar::when_all(p->begin(), p->end()).then(
        [p] (std::vector<seastar::future<uint64_t>> ret) {
            std::uint64_t sum = 0;
            for (auto& r: ret) {
                sum += r.get0();            
            }
            return sum;
        }
    );
}

using clock_type = std::chrono::steady_clock;
using duration_type = clock_type::duration;
using time_point_type = clock_type::time_point;

int main(int argc, char** argv) {
    seastar::app_template app;
    app.run(argc, argv, [] {
        std::cout << "Paralellism factor: " << seastar::smp::count << "\n";

        const std::size_t size{ 10'000'000 };
        const std::size_t div{ 10 };

        time_point_type start{ clock_type::now() };

        return skynet_start(0, size, div).then(
            [start] (std::uint64_t result) {
                std::cout << result << std::endl;
                auto duration = clock_type::now() - start;
                std::cout << "duration: " << duration.count() / 1'000'000 << " ms" << std::endl;

                if (49999995000000 != result) {
                    throw std::runtime_error("invalid result");
                }
            }
        );
        // return seastar::check_direct_io_support("/home/max")
        // .then([] {
        //     std::cout << "Ok, homedir is with DMA support" << std::endl;
        // }).handle_exception([](auto exception_ptr){
        //     std::cout << "Homdir is NOT support DMA" << std::endl;
        // }).then([] {
        //     return seastar::file_system_at("/");
        // }).then([] (auto fs_type) {
        //     std::cout << std::boolalpha << "FS type is Ext4: " << (fs_type == seastar::fs_type::ext4) << std::endl;
        // }).then([] {
        //     return f();
        //     // return seastar::async([] {
        //     //     std::cout << seastar::thread::running_in_thread() << std::endl;
        //     // });
        // });
    });
}