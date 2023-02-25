#include "v2c.hpp"

map<int, bool> keyCodeTrackerLIB;
const int keyCheckFreq = 60;
bool end_ = false;

void KeyCodeTracker()
{
    register int keycodeTrackerSleep = 1000 / keyCheckFreq;
    while (1)
    {
        Sleep(keycodeTrackerSleep);
        if (end_)
        {
            return;
        }
        map<int, bool>::iterator iter;
        iter = keyCodeTrackerLIB.begin();
        while (iter != keyCodeTrackerLIB.end())
        {
            if (!KEY_DOWN(iter->first))
            {
                keyCodeTrackerLIB.erase(iter->first);
                break;
            }
            iter++;
        }
    }
    return;
}
inline bool keyDown(int KeyCode)
{
    return KEY_DOWN(KeyCode);
}
inline bool keyPress(int KeyCode)
{
    if (keyDown(KeyCode))
    {
        bool used = false;
        if (keyCodeTrackerLIB.count(KeyCode) == 0)
        {
            keyCodeTrackerLIB.insert({ KeyCode,true });
            return true;
        }
    }
    return false;
}
inline void fatal(const char* err, bool code = false)
{
    cerr << "FATAL ERROR\n\n";
    cerr << "This program ran into a error that it couldn't handle,\n";
    cerr << "and has to exit right now.";
    if (code)
    {
        cerr << "error-code: " << int(err[0] - '0');
    }
    else
    {
        cerr << "error-info: ";
        puts(err);
    }
    exit(err[0]);
}
void usage()
{
    cout << "Usage:" << endl;
    cout << "\tv2c.exe <target> <type>" << endl;
    cout << endl;
    cout << "<target>\tThe file(video or image) path for processing." << endl;
    cout << "<type>\tThe file type. Video: \"-v\", Image:\"-i\".";
    cout << endl;
}

inline bool inear(float a, float b, float dif = 0.1)
{
    return (a >= b && a - dif <= b);
}

vector<string> img2console(Mat img, int zoom, float threshold)
{
    const float factor = 1.0 * img.cols / img.rows * 2.0;
    float lengthX = zoom;
    float lengthY = zoom / factor;
    float pixelPerCharX = img.cols / lengthX;
    float pixelPerCharY = img.rows / lengthY;
    lengthX = ceil(lengthX);
    lengthY = ceil(lengthY);
    pixelPerCharX = ceil(pixelPerCharX);
    pixelPerCharY = ceil(pixelPerCharY);
    vector<string> frame;
    register string buffer;
    register double val;
    register double average;

    for (int i = 0; i < lengthY; ++i)
    {
        buffer.clear();
        for (int j = 0; j < lengthX; ++j)
        {
            val = 0;
            for (int k = i * pixelPerCharY; k < (i + 1) * pixelPerCharY; ++k)
            {
                for (int l = j * pixelPerCharX; l < (j + 1) * pixelPerCharX; ++l)
                {
                    if (l >= img.cols || k >= img.rows) { continue; };
                    val += (img.at<uchar>({ l,k })) / 127.5;
                }
            }
            average = 1.0 * val / (pixelPerCharX * pixelPerCharY) * threshold;

            if (average > 1.1)
            {
                buffer += '@';
            }
            if (inear(average, 1))
            {
                buffer += '#';
            }
            else if (inear(average, 0.9))
            {
                buffer += '%';
            }
            else if (inear(average, 0.8))
            {
                buffer += '9';
            }
            else if (inear(average, 0.7))
            {
                buffer += 'M';
            }
            else if (inear(average, 0.6))
            {
                buffer += 'H';
            }
            else if (inear(average, 0.5))
            {
                buffer += '?';
            }
            else if (inear(average, 0.4))
            {
                buffer += '=';
            }
            else if (inear(average, 0.3))
            {
                buffer += '/';
            }
            else if (inear(average, 0.2))
            {
                buffer += ':';
            }
            else if (inear(average, 0.1))
            {
                buffer += '.';
            }
            else if (inear(average, 0))
            {
                buffer += ' ';
            }
        }
        frame.push_back(buffer);
    }
    return frame;
}
void video2console(int fps, unsigned long long totalFrames, VideoCapture cap, int zoom, float threshold)
{
    clock_t start, end;
    double total = 0, average;
    unsigned long long etc = 0x3f3f3f;
    Mat frame, tmp;
    int cnt = 0;
    vector<string> frameOut;
    fstream fout;
    fout.open(".\\output.txt", ios::out);
    fout << fps << endl;
    clog << "\n\n";
    for (int nowCnt = 0; true; ++nowCnt)
    {
        start = clock();
        cap >> tmp;
        if (tmp.empty())
        {
            break;
        }
        cvtColor(tmp, frame, COLOR_BGR2GRAY);
        frameOut = img2console(frame, zoom, threshold);

        for (int i = 0; i < frameOut.size(); ++i)
        {
            fout << frameOut[i] << endl;
        }
        fout << "[ENDFRAME]\n";
        fout << flush;
        end = clock();
        total += end - start;
        average = 1.0 * total / ++cnt / CLOCKS_PER_SEC;
        if (cnt % 10 == 0)
        {
            system("cls");
            clog << round(1.0 * cnt / totalFrames * 100) << "% done,\t"
                << (long long)round(average * (totalFrames - cnt)) % 3600 / 60 << "Min & "
                << (long long)round(average * (totalFrames - cnt)) % 60 << " Sec etc.\t"
                << "(" << cnt << "/" << totalFrames << " Frames)"
                << endl;
            clog << "[";
            for (int i = 0; i < round(1.0 * cnt / totalFrames * 100); ++i)
            {
                clog << "=";
            }
            for (int i = round(1.0 * cnt / totalFrames * 100); i < 100; ++i)
            {
                clog << " ";
            }
            clog << "]";
        }
    }
    fout.close();
    return;
}

void ui(string filePath = "", char fileType = ' ')
{
    thread keyCodeTrackerProcess(KeyCodeTracker);

    int size = 100;
    float thres = 1;
    char filePathcstr[1024] = "\0";
    vector<string> frame;

    if (filePath.empty())
    {
        cout << "Please enter the path of targeting file: ";
        gets_s(filePathcstr);
        filePath = filePathcstr;
        while (fileType != 'v' && fileType != 'i')
        {
            cout << "Please enter the type of targeting file(Video:\"v\", Image:\"i\"): ";
            cin >> fileType;
        }
    }

    VideoCapture cap;
    Mat tmp, now;
    int Currentframe = 1;
    if (fileType == 'v')
    {
        if (keyCodeTrackerLIB.count(VK_RETURN) == 0)
        {
            keyCodeTrackerLIB.insert({ VK_RETURN,true });
        }
    _v_:
        cap.release();
        cap.open(filePath);
        for (int i = 0; i < Currentframe; ++i)
        {
            cap >> tmp;
        }
        cvtColor(tmp, now, COLOR_BGR2GRAY);
        while (true)
        {
            system("cls");
            frame = img2console(now, size, thres);
            for (int i = 0; i < frame.size(); ++i)
            {
                cout << frame[i] << endl;
            }
            cout << "use direction arrows to control scale and threshold, use Enter to execute\n";
            cout << "use F1 and F2 to control the preview video position. Current position: Frame #" << Currentframe << endl;
            cout << "Scale: " << size << endl << "Exposure: " << thres;

            while (!(keyPress(VK_UP) || keyPress(VK_DOWN) || keyPress(VK_LEFT) || keyPress(VK_RIGHT) || keyPress(VK_RETURN) || keyPress(VK_F1) || keyPress(VK_F2)))
            {
                waitKey(50);
            }
            if (keyDown(VK_UP))
            {
                size += 10;
            }
            if (keyDown(VK_DOWN))
            {
                size -= 10;
            }
            if (keyDown(VK_LEFT))
            {
                thres -= 0.1;
            }
            if (keyDown(VK_RIGHT))
            {
                thres += 0.1;
            }
            if (keyDown(VK_F1))
            {
                Currentframe = max(1, Currentframe - 30);
                goto _v_;
            }
            if (keyDown(VK_F2))
            {
                Currentframe = min((int)cap.get(CAP_PROP_FRAME_COUNT), Currentframe + 30);
                for (int i = 0; i < 30; ++i)
                {
                    cap >> tmp;
                }
                cvtColor(tmp, now, COLOR_BGR2GRAY);
            }
            if (keyDown(VK_RETURN))
            {
                break;
            }
        }
        cap.release();
        cap.open(filePath);
        video2console(cap.get(CAP_PROP_FPS), cap.get(CAP_PROP_FRAME_COUNT), cap, size, thres);
    }
    else
    {
        now = imread(filePath, IMREAD_GRAYSCALE);

        if (keyCodeTrackerLIB.count(VK_RETURN) == 0)
        {
            keyCodeTrackerLIB.insert({ VK_RETURN,true });
        }
        while (true)
        {
            system("cls");
            frame = img2console(now, size, thres);
            for (int i = 0; i < frame.size(); ++i)
            {
                cout << frame[i] << endl;
            }
            cout << "use direction arrows to control scale and threshold, use Enter to execute\n";
            cout << "Scale: " << size << endl << "Exposure: " << thres;
            while (!(keyPress(VK_UP) || keyPress(VK_DOWN) || keyPress(VK_LEFT) || keyPress(VK_RIGHT) || keyPress(VK_RETURN)))
            {
                waitKey(50);
            }
            if (keyDown(VK_UP))
            {
                size += 10;
            }
            if (keyDown(VK_DOWN))
            {
                size -= 10;
            }
            if (keyDown(VK_LEFT))
            {
                thres -= 0.1;
            }
            if (keyDown(VK_RIGHT))
            {
                thres += 0.1;
            }
            if (keyDown(VK_RETURN))
            {
                break;
            }
        }
        system("cls");
        frame = img2console(now, size, thres);
        for (int i = 0; i < frame.size(); ++i)
        {
            cout << frame[i] << endl;
        }
    }

    end_ = true;
    if (keyCodeTrackerProcess.joinable())
    {
        keyCodeTrackerProcess.join();
    }
    return;
}

int main(int argc, char *argv[])
{
    string argv2;
    if (argc > 2)
    {
         argv2 = argv[2];
    }
    switch (argc)
    {
    case 1:
        ui();
        break;
    case 2:
        ui(argv[1], 'v');
        break;
    case 3:
        if (argv2 == "-v" || argv2 == "-V")
        {
            ui(argv[1], 'v');
        }
        else if (argv2 == "-i" || argv2 == "-I")
        {
            ui(argv[1], 'i');
        }
        else
        {
            fatal("2");
        }
        break;
    default:
        cout << "Too few or too many Arguments!\n\n\n";
        usage();
        fatal("1");
        break;
    }
    return 0;
}
