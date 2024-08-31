#include "vkrun/vkrun.hpp"
#include "laser/laser.hpp"
#include "keyinput/keyinput.hpp"
#include "lvx_file/lvx_file.hpp"
#include "las_file/las_file.hpp"
#include <thread>

static bool onExit = false;
bool run = false;

void keyinput_run()
{
    run = true;
    return;
}

void fraps_main(HVKApp *app)
{
    auto tt = std::chrono::steady_clock::now();
    uint32_t prev_count = 0, cur_count = 0;
    // while (!run)
    //     ;
    while (!onExit)
    {
        cur_count = app->getFrameCount();
        std::cout << cur_count - prev_count << "\n";
        prev_count = cur_count;
        tt += std::chrono::seconds(1);
        std::this_thread::sleep_until(tt);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "File name\n";
        return 0;
    }
    // read_laser_csv();
    read_lvx_file(argv[1]);
    // read_las_file(argv[1]);
    HVKApp app;
    std::thread fraps(fraps_main, &app);

    try
    {
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    onExit = true;
    fraps.join();
    return EXIT_SUCCESS;
}
