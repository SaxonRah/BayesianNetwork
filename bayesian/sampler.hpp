#ifndef BNI_SAMPLER_HPP
#define BNI_SAMPLER_HPP

#include <fstream>
#include <sstream>
#include <functional>
#include <vector>
#include <numeric>
#include <unordered_map>
#include <boost/optional.hpp>
#include <boost/algorithm/string.hpp>
#include <bayesian/graph.hpp>
#include <bayesian/hash.hpp>

namespace bn {

class sampler {
public:
    sampler()
        : filename_(), table_(), sampling_size_(0)
    {
    }

    sampler(std::string const& filename)
        : filename_(filename), table_(), sampling_size_(0)
    {
    }

    bool load_sample(std::unordered_map<condition_t, std::size_t> const& table)
    {
        table_ = table;

        sampling_size_ = 0;
        for(auto const& p : table) sampling_size_ += p.second;

        return true;
    }

    // 事前に設定したファイルからCSVデータを読み込み，一般化したテーブルを作る
    // 引数: node_list: ファイルに記述されている順番を指定するvector
    // 戻り値: 成功した場合のみ true
    bool load_sample(std::vector<vertex_type> const& node_list)
    {
        // ファイルオープン
        std::ifstream ifs(filename_);
        if(!ifs.is_open()) return false;

        // 読み込み先
        std::size_t sampling_size = 0;
        std::unordered_map<condition_t, std::size_t> table;

        // 1行ずつサンプリング
        std::string line_str;
        condition_t sample;
        while(std::getline(ifs, line_str))
        {
            std::vector<std::string> line;
            boost::algorithm::split(line, line_str, boost::is_space(), boost::algorithm::token_compress_on);

            auto select_it = line.cbegin() + 1;
            for(auto it = node_list.cbegin(); it != node_list.cend();)
                sample[*it++] = std::stoi(*select_it++);

            auto const sample_num = std::stoi(line[0]);

            auto const it = table.find(sample);
            if(it == table.cend()) table[sample] = sample_num;
            else (it->second) += sample_num;

            sampling_size += sample_num;
        }

        sampling_size_ = sampling_size;
        table_ = std::move(table);
        return true;
    }

	// load_sampleによって生成したテーブルから，CPTを計算し，graphに格納する
	// 引数: graph: 読み込みしたCPTを保存するグラフ構造
	// 戻り値: 成功した場合のみ true
    bool make_cpt(graph_t const& graph) const
    {
        // sampleが読み込まれているかどうか
        if(sampling_size() == 0) return false;

        // CPTの初期化
        for(auto const& node : graph.vertex_list())
        {
            auto const in_vertex = graph.in_vertexes(node);
            node->cpt.assign(in_vertex, node);
        }

        // カウンタ
        std::unordered_map<
            std::pair<vertex_type, condition_t>,
            std::vector<std::size_t>,
            bn::hash<std::pair<vertex_type, condition_t>>
        > counter;

        // カウント
        for(auto const& sample : table())
        {
            for(auto const& node : graph.vertex_list())
            {
                auto const parents = graph.in_vertexes(node);

                // 条件を畳み込む
                condition_t conditional;
                for(auto const& parent : parents) conditional[parent] = sample.first.at(parent);

                // counterのカウントアップ
                auto container_first = std::make_pair(node, conditional);
                auto const it = counter.find(container_first);
                if(it == counter.end())
                {
                    // 作ってcounterに要素を追加
                    std::vector<std::size_t> local_counter(node->selectable_num, 0);
                    local_counter[sample.first.at(node)] = sample.second;
                    counter.insert(std::make_pair(std::move(container_first), std::move(local_counter)));
                }
                else
                {
                    // 足しあわせ
                    it->second[sample.first.at(node)] += sample.second;
                }
            }
        }

        // カウント値を用い，CPTを算出
        for(auto const& node : graph.vertex_list())
        {
            for(auto const& conditional : node->cpt.pattern())
            {
                // カウンタからデータを引きずりだし，確率1にすべき母数を算出
                auto const key = std::make_pair(node, conditional);
                auto const it = counter.find(key);
                auto const select_counter =
                    (it != counter.end()) ? counter.at(std::make_pair(node, conditional))
                                          : std::vector<std::size_t>(node->selectable_num, 0);

                double const parameter = static_cast<double>(std::accumulate(select_counter.begin(), select_counter.end(), static_cast<std::size_t>(0)));

                // 本題のCPT作成
                std::vector<double> cpt(node->selectable_num);
                if(parameter == 0)
                {
                    // 同じ値を与える
                    for(std::size_t i = 0; i < node->selectable_num; ++i)
                        cpt[i] = /*0.0;*/static_cast<double>(1.0) / node->selectable_num;
                }
                else
                {
                    // CPTを正規化する
                    for(std::size_t i = 0; i < node->selectable_num; ++i)
                        cpt[i] = static_cast<double>(select_counter.at(i)) / parameter;
                }

                node->cpt[conditional].second = std::move(cpt);
            }
        }

        return true;
    }

    // 読み込み対象のファイルを指定するgetter
    std::string filename() const
    {
        return filename_;
    }

    // 読み込み対象のファイルを指定するsetter
    void set_filename(std::string const& filename)
    {
        filename_ = filename;
        sampling_size_ = 0;
        table_.clear();
    }

	// テーブルに関するgetter
	std::unordered_map<condition_t, std::size_t> table() const
    {
        return table_;
    }

	// サンプル数に関するgetter
    std::size_t sampling_size() const
    {
        return sampling_size_;
    }

private:
    std::string filename_;
    std::unordered_map<condition_t, std::size_t> table_;
    std::size_t sampling_size_;

    // 与えられた確率変数全ての組み合わせに対し，functionを実行するというインターフェースを提供する
    // TODO: class bpの中と重複しているので，後で整理する
    void all_combination_pattern(
        std::vector<vertex_type> const& combination,
        std::function<void(condition_t const&)> const& function
        )
    {
        typedef std::vector<vertex_type>::const_iterator iterator_type;
        std::function<void(iterator_type const, iterator_type const&)> recursive;

        condition_t condition;
        recursive = [&](iterator_type const it, iterator_type const& end)
        {
            if(it == end)
            {
                function(condition);
            }
            else
            {
                for(int i = 0; i < (*it)->selectable_num; ++i)
                {
                    condition[*it] = i;
                    recursive(it + 1, end);
                }
            }
        };

        recursive(combination.cbegin(), combination.cend());
    }
};

} // namespace bn

#endif // #ifndef BNI_SAMPLER_HPP
