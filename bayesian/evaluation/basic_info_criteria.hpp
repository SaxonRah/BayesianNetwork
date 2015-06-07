#ifndef BNI_EVALUATION_BASIC_INFO_CRITERIA_HPP
#define BNI_EVALUATION_BASIC_INFO_CRITERIA_HPP

#include <cstdint>
#include <numeric>
#include <bayesian/graph.hpp>
#include <bayesian/sampler.hpp>
#include <bayesian/evaluation/basic_evaluation.hpp>

namespace bn {
namespace evaluation {

class basic_info_criteria : basic_evaluation{
public:
    basic_info_criteria(sampler const& sampling);

    // API
    virtual double operator() (graph_t const& graph) const
    {
        return (*this)(graph, graph.vertex_list());
    }
    virtual double operator() (graph_t const& graph, std::vector<bn::vertex_type> const& vertex_list) const = 0;

protected:
    // AIC/MDL�������: �l�b�g���[�N�\���̓K�؂��Ɋւ��鍀
    // - log P_theta^N(D)
    double calc_likelihood(graph_t const& graph) const;
    double calc_likelihood(graph_t const& graph, std::vector<bn::vertex_type> const& vertex_list) const;

    // AIC/MDL�������: �l�b�g���[�N�\���̕��G���Ɋւ��鍀
    // + d
    double calc_parameters(graph_t const& graph) const;

    // Sampling����getter
    std::size_t sampling_size() const;

private:
    sampler const& sampling_;
};

basic_info_criteria::basic_info_criteria(sampler const& sampling)
    : sampling_(sampling)
{
}

double basic_info_criteria::calc_likelihood(graph_t const& graph) const
{
    return calc_likelihood(graph, graph.vertex_list());
}

double basic_info_criteria::calc_likelihood(graph_t const& graph, std::vector<bn::vertex_type> const& vertex_list) const
{
    double likelihood = 0.0;

    // �e�m�[�h�ɑ΂��ޓx�̌v�Z
    for(auto const& node : vertex_list)
    {
        // �e�m�[�h
        auto const& parent = graph.in_vertexes(node);

        // �e�T���v���񂵂ē��v�����(�������������̂��܂Ƃߏグ�銴�o)
        std::unordered_map<condition_t, std::size_t> statistics;
        for(auto const& sample : sampling_.table())
        {
            // ���m�[�h�Ɛe�m�[�h�ȊO������������
            condition_t cond = sample.first;
            for(auto it = cond.begin(); it != cond.end();)
            {
                if(it->first != node && std::find(parent.begin(), parent.end(), it->first) == parent.end())
                    it = cond.erase(it);
                else
                    ++it;
            }

            statistics[cond] += sample.second;
        }

        // ���v���{���̌v�Z
        for(auto data : statistics)
        {
            auto cond = data.first;
            auto const select = cond.at(node);
            cond.erase(node);
            likelihood -= data.second * std::log(node->cpt[cond].second[select]);
        }
    }

    return likelihood;
}

double basic_info_criteria::calc_parameters(graph_t const& graph) const
{
    int64_t parameters = 0;
    for(auto const& node : graph.vertex_list())
    {
        auto const& parents = graph.in_vertexes(node);

        // 1�m�[�h�̃p�����[�^���𑫂����킹��
        parameters += std::accumulate(
            parents.begin(), parents.end(), node->selectable_num - 1,
            [](int64_t const& init, bn::vertex_type const& parent_node) -> int64_t
            {
                return init * parent_node->selectable_num;
            });
    }

    return static_cast<double>(parameters);
}

std::size_t basic_info_criteria::sampling_size() const
{
    return sampling_.sampling_size();
}

} // namespace evaluation
} // namespace bn

#endif // #ifndef BNI_EVALUATION_BASIC_INFO_CRITERIA_HPP
