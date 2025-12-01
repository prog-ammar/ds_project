#include<iostream>
#include<vector>
#include<list>
#include<algorithm>
#include<map>
#include<math.h>
#include<sstream>
#include<fstream>

using namespace std;

struct Song
{
    string id;
    string title;
    string artist;
    string genre;
    double loudness;
    double energy;
    double tempo;
    int duration;
    double accoust;
    double danceibility;

};


double normalize_value(double min_v, double max_v, double v)
{
    return (v - min_v) / (max_v - min_v);
}


struct Vertice
{
    int in_degree;
    int out_degree;
    map<string, pair<double, Vertice*>> adj_vertices;
    int data;
    string id;
    double weight;

    Vertice(string id = "", int data = 0)
    {
        this->id = id;
        this->data = data;
        in_degree = out_degree = 0;
    }
};

class Recom_Graph
{
private:
    map<string, Vertice*> vertices;

public:

    void add_vertice(string src, string dest, double weight)
    {
        if (vertices.find(src) == vertices.end())
        {
            Vertice* src_vertice = new Vertice(src);
            vertices[src] = src_vertice;
        }
        if (vertices.find(dest) == vertices.end())
        {
            Vertice* dest_vertice = new Vertice(dest);
            vertices[dest] = dest_vertice;
        }

        if (vertices[src]->adj_vertices.count(dest) == 0)
        {
            vertices[src]->adj_vertices.insert({ dest,{weight,vertices[dest]} });
            vertices[dest]->adj_vertices.insert({ src,{weight,vertices[src]} });

            vertices[src]->in_degree++;
            vertices[src]->out_degree++;
            vertices[dest]->in_degree++;
            vertices[dest]->out_degree++;
        }
    }

    void print(string id)
    {
        for (auto i : vertices[id]->adj_vertices)
        {
            cout << i.first << " " << i.second.first << endl;
        }
    }

    string recommend(string id)
    {
        string max_sim_id;
        double max_sim = 0;;
        for (auto i : vertices[id]->adj_vertices)
        {
            if (i.second.first > max_sim)
            {
                max_sim = i.second.first;
                max_sim_id = i.first;
            }
        }
        return max_sim_id;
    }
};

class Player
{
private:
    map<string, Song> songs;
    map<string, vector<Song>> playlists_by_genre;
    map<string, vector<Song>> playlists_by_artist;
    Recom_Graph graph;

public:
    void read_from_file(string filename)
    {
        ifstream file;
        file.open(filename);
        string whole_line;
        string s_details[10];
        Song song;
        int row = 0;
        while (getline(file, whole_line))
        {
            if (row == 0)
            {
                row++;
                continue;
            }
            stringstream line(whole_line);
            for (int i = 0;i < 10;i++)
            {
                getline(line, s_details[i], ',');
            }
            song.id = s_details[0];
            song.title = s_details[1];
            song.artist = s_details[2];
            song.genre = s_details[3];
            song.loudness = stod(s_details[4]);
            song.energy = stod(s_details[5]);
            song.tempo = stod(s_details[6]);
            song.duration = stoi(s_details[7]);
            song.accoust = stod(s_details[8]);
            song.danceibility = stod(s_details[9]);
            playlists_by_genre[song.genre].push_back(song);
            playlists_by_artist[song.artist].push_back(song);
            row++;
            songs[song.id] = song;
        }
    }

    void print_playlist_by_genre()
    {
        for (auto& i : playlists_by_genre)
        {
            cout << i.first << "\n\n";
            for (auto j : i.second)
            {
                cout << "ID : " << j.id << endl;
                cout << "Title : " << j.title << endl;
                cout << "Artist : " << j.artist << endl;
                cout << "Genre : " << j.genre << endl;
                cout << "Loudness : " << j.loudness << endl;
                cout << "Energy : " << j.energy << endl;
                cout << "Tempo : " << j.tempo << endl;
                cout << "Duration : " << j.duration << endl;
                cout << "Accousti : " << j.accoust << endl;
                cout << "Dancibility : " << j.danceibility << endl;
                cout << endl;
            }
        }
    }

    void print_playlist_by_artist()
    {
        for (auto& i : playlists_by_artist)
        {
            cout << i.first << "\n\n";
            for (auto j : i.second)
            {
                cout << "ID : " << j.id << endl;
                cout << "Title : " << j.title << endl;
                cout << "Artist : " << j.artist << endl;
                cout << "Genre : " << j.genre << endl;
                cout << "Loudness : " << j.loudness << endl;
                cout << "Energy : " << j.energy << endl;
                cout << "Tempo : " << j.tempo << endl;
                cout << "Duration : " << j.duration << endl;
                cout << "Accousti : " << j.accoust << endl;
                cout << "Dancibility : " << j.danceibility << endl;
                cout << endl;
            }
        }
    }

    void print_details()
    {
        for (auto& i : songs)
        {
            cout << "ID : " << i.second.id << endl;
            cout << "Title : " << i.second.title << endl;
            cout << "Artist : " << i.second.artist << endl;
            cout << "Genre : " << i.second.genre << endl;
            cout << "Loudness : " << i.second.loudness << endl;
            cout << "Energy : " << i.second.energy << endl;
            cout << "Tempo : " << i.second.tempo << endl;
            cout << "Duration : " << i.second.duration << endl;
            cout << "Accousti : " << i.second.accoust << endl;
            cout << "Dancibility : " << i.second.danceibility << endl;
            cout << endl;
        }
    }

    void calculate_similarity_of_all_songs()
    {
        for (auto& j : songs)
        {
            for (auto& k : songs)
            {
                if (j.first != k.first)
                {
                    calculate_similarity_of_songs(j.second, k.second);
                }
            }
        }

    }

    void calculate_similarity_of_songs(Song A, Song B)
    {
        vector<double> vectorA;
        vectorA.push_back(normalize_value(0, 1, A.loudness));
        vectorA.push_back(normalize_value(0, 1, A.tempo));
        vectorA.push_back(normalize_value(0, 1, A.energy));
        vectorA.push_back(normalize_value(0, 1, A.accoust));
        vectorA.push_back(normalize_value(0, 1, A.danceibility));

        vector<double> vectorB;
        vectorB.push_back(normalize_value(0, 1, B.loudness));
        vectorB.push_back(normalize_value(0, 1, B.tempo));
        vectorB.push_back(normalize_value(0, 1, B.energy));
        vectorB.push_back(normalize_value(0, 1, B.accoust));
        vectorB.push_back(normalize_value(0, 1, B.danceibility));

        double dot_product = 0;
        double sqrA = 0;
        double sqrB = 0;

        for (int i = 0;i < vectorA.size();i++)
        {
            dot_product += vectorA[i] * vectorB[i];
            sqrA += vectorA[i] * vectorA[i];
            sqrB += vectorB[i] * vectorB[i];
        }

        double cosine_similarity = dot_product / ((sqrt(sqrA)) * (sqrt(sqrB)));

        double genre_factor = 0.1;
        double artist_factor = 0.33;
        double Cosine_factor = 0.57;

        double genre_v = (A.genre == B.genre) ? 1 : 0;
        double artist_v = (A.artist == B.artist) ? 1 : 0;

        double final = (Cosine_factor * cosine_similarity) + (genre_factor * genre_v) + (artist_v * artist_factor);

        if (final >= 0.5)
            graph.add_vertice(A.id, B.id, final);

    }


    void print()
    {
        cout << songs[graph.recommend("R01")].id << endl;
        cout << songs[graph.recommend("R14")].id << endl;
    }

};

//int main()
//{
//    Player player;
//    player.read_from_file("songs_set.csv");
//    player.calculate_similarity_of_all_songs();
//    player.print();
//
//    // player.print_playlist_by_artist();
//}