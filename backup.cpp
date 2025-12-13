#include<iostream>
#include<vector>
#include<list>
#include<algorithm>
#include<map>
#include<math.h>
#include<sstream>
#include<fstream>
#include<algorithm>

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

    bool operator==(const Song& other) const
    {
        return other.id == id;
    }

};

inline double normalize_value(double min_v, double max_v, double v)
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

struct trienode {
    map<char, trienode*> children;
    vector<string> song_ids;
    bool is_end = false;
};

class tire {
private:
    trienode* root;

public:
    tire() {
        root = new trienode();
    }

    // Insert a song title into the trie
    void insert(const string& song_title, const string& song_id) {
        trienode* node = root;
        for (char c : song_title) {
            c = tolower(c); // make search case-insensitive
            if (!node->children.count(c)) {
                node->children[c] = new trienode();
            }
            node = node->children[c];
            node->song_ids.push_back(song_id); // store song ID at every prefix
        }
        node->is_end = true;
    }

    // Search songs by prefix
    vector<string> search(const string& prefix) {
        trienode* node = root;
        for (char c : prefix) {
            c = tolower(c);
            if (!node->children.count(c))
                return {}; // no match
            node = node->children[c];
        }
        return node->song_ids; // all songs with this prefix
    }
};

class Player
{
private:
    map<string, Song> songs;
    map<string, vector<string>> playlists_by_genre;
    map<string, vector<string>> playlists_by_artist;
    map<string, vector<string>> user_playlist;
    Recom_Graph graph;

    tire song_trie;


    void build_trie()
    {
        for (auto& s : songs)
        {
            string formatted_title = s.second.title;
            replace(formatted_title.begin(), formatted_title.end(), '_', ' ');
            song_trie.insert(formatted_title, s.second.id);
        }
    }

    void initialize_search()
    {
        build_trie();
    }

public:

    Player()
    {
        read_from_file("songs_set_1.csv");
        read_user_playlist("user_playlists.csv");
        build_trie();
    }

    void read_user_playlist(string filename)
    {
        ifstream file(filename);
        string wholeline;
        int row = 0;
        string p_name = "";
        string s_name = "";
        while (getline(file, wholeline))
        {
            if (row == 0)
            {
                row++;
                continue;
            }

            stringstream line(wholeline);
            getline(line, p_name, ',');
            while (getline(line, s_name, ','))
            {
                if (!s_name.empty())
                    user_playlist[p_name].push_back(s_name);
            }
            row++;
        }
        file.close();
    }



    void write_user_playlist(string filename)
    {
        ofstream file(filename);
        file << "Playlist Name" << "," << "Songs IDs" << "\n";
        for (auto& i : user_playlist)
        {
            file << i.first << ",";
            for (auto& j : i.second)
            {
                file << j << ",";
            }
            file << "\n";
        }
        file.close();
    }

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
            for (int i = 0; i < 10; i++)
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
            playlists_by_genre[song.genre].push_back(song.id);
            playlists_by_artist[song.artist].push_back(song.id);
            row++;
            songs[song.id] = song;
        }
        file.close();
    }

    void add_song(string playlist_name, string song_id)
    {
        if (find(user_playlist[playlist_name].begin(), user_playlist[playlist_name].end(), song_id) == user_playlist[playlist_name].end())
        {
            user_playlist[playlist_name].push_back(song_id);
        }
    }

    void remove_song(string playlist_name, string song_id)
    {
        if (user_playlist.count(playlist_name) == 0)
        {
            cout << "Sorry :( Playlist " << playlist_name << " does not exist" << endl;
            return;
        }

        vector <string>& p = user_playlist[playlist_name];
        p.erase(remove(p.begin(), p.end(), song_id), p.end());
    }

    Song get_song(string song_id)
    {
        return songs[song_id];
    }

    map<string, vector<string>> get_genre()
    {
        return playlists_by_genre;
    }

    map<string, vector<string>> get_artist()
    {
        return playlists_by_artist;
    }

    void print_playlist_by_genre()
    {
        for (auto& i : playlists_by_genre)
        {
            cout << i.first << "\n\n";
            for (auto j : i.second)
            {
                Song s = get_song(j);
                cout << "ID : " << s.id << endl;
                cout << "Title : " << s.title << endl;
                cout << "Artist : " << s.artist << endl;
                cout << "Genre : " << s.genre << endl;
                cout << "Loudness : " << s.loudness << endl;
                cout << "Energy : " << s.energy << endl;
                cout << "Tempo : " << s.tempo << endl;
                cout << "Duration : " << s.duration << endl;
                cout << "Accousti : " << s.accoust << endl;
                cout << "Dancibility : " << s.danceibility << endl;
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
                Song s = get_song(j);
                cout << "ID : " << s.id << endl;
                cout << "Title : " << s.title << endl;
                cout << "Artist : " << s.artist << endl;
                cout << "Genre : " << s.genre << endl;
                cout << "Loudness : " << s.loudness << endl;
                cout << "Energy : " << s.energy << endl;
                cout << "Tempo : " << s.tempo << endl;
                cout << "Duration : " << s.duration << endl;
                cout << "Accousti : " << s.accoust << endl;
                cout << "Dancibility : " << s.danceibility << endl;
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

        for (int i = 0; i < vectorA.size(); i++)
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

    // New accessors for GUI usage
    const map<string, vector<string>>& get_user_playlists() const
    {
        return user_playlist;
    }

    vector<string> get_user_playlist(const string& name) const
    {
        auto it = user_playlist.find(name);
        if (it != user_playlist.end())
            return it->second;
        return {};
    }

    // Add a convenience to create an empty playlist (no-op if exists)
    void create_playlist(const string& name)
    {
        if (user_playlist.find(name) == user_playlist.end())
            user_playlist[name] = {};
    }

    // Delete playlist from map and persist change to file
    void delete_playlist(const string& name)
    {
        // erase from in-memory map
        auto erased = user_playlist.erase(name);

        // rewrite backing file to reflect change regardless of whether key existed
        write_user_playlist("user_playlists.csv");
    }

    vector<string> search_songs_by_prefix(const string& prefix) {
        vector<string> ids = song_trie.search(prefix);
        return ids;
    }


};

struct Node
{
    Node* next;
    Node* prev;
    string song_id;

     Node(string song_id)
    {
        this->next = NULL;
        this->prev = NULL;
        this->song_id = song_id;
    }
};

class Circular_Doubly_Linked_List
{
private:
    Node* head;
    Node* tail;
    Node* curr;

public:
    Circular_Doubly_Linked_List()
    {
        head = tail = curr = NULL;
    }

    void add_song_at_tail(string song_id)
    {
        Node* new_node = new Node(song_id);
        if (head == NULL && tail == NULL)
        {
            tail = new_node;
            head = tail;
            head->next = tail;
            tail->prev = head;

            return;
        }
        new_node->prev = tail;
        tail->next = new_node;
        new_node->next = head;
        head->prev = new_node;
        tail = new_node;
    }

    void add_song_at_head(string song_id)
    {
        Node* new_node = new Node(song_id);
        if (head == NULL && tail == NULL)
        {
            tail = new_node;
            head = tail;
            head->next = tail;
            tail->prev = head;
            return;
        }

        new_node->next = head;
        head->prev = new_node;
        tail->next = new_node;
        new_node->prev = tail;
        head = new_node;
    }


    bool isEmpty()
    {
        return head == NULL && tail == NULL;
    }

    void Clear()
    {
        Node* temp = head;
        do
        {
            Node* temp1 = temp->next;
            delete temp;
            temp = temp1;
        } while (temp != tail);
        delete temp;
        head = tail = NULL;
    }
    
    string start_curr()
    {
        if (head != NULL)
        {
            curr = head;
            return curr->song_id;
        }
        cout << "Error : No Playlist is initialized\n";
        return "";
    }

    string move_curr_front()
    {
        if (!isEmpty())
        {
            curr = curr->next;
            return curr->song_id;
        }
        cout << "Error : Playlist is Empty\n";
        return "";
        
    }

    string move_curr_back()
    {
        if (!isEmpty())
        {
            curr = curr->prev;
            return curr->song_id;
        }
        cout << "Error : Playlist is Empty\n";
        return "";
    }

};


kesa hy qazi

