#ifndef BNI_EVALUATION_BASIC_INFO_CRITERIA_HPP
#define BNI_EVALUATION_BASIC_INFO_CRITERIA_HPP

#include <cstdint>
#include <numeric>
#include <boost/optional.hpp>
#include <bayesian/graph.hpp>
#include <bayesian/sampler.hpp>
#include <bayesian/evaluation/basic_evaluation.hpp>

namespace bn {
namespace evaluation {

class basic_info_criteria : basic_evaluation{
public:
    basic_info_criteria(std::string const& file);

    // API
    virtual double operator() (graph_t const& graph) const = 0;

protected:
    // AIC/MDL�������: �l�b�g���[�N�\���̓K�؂��Ɋւ��鍀
    // - log P_theta^N(D)
    double calc_likelihood(graph_t const& graph) const;

    // AIC/MDL�������: �l�b�g���[�N�\���̕��G���Ɋւ��鍀
    // + d
    double calc_parameters(graph_t const& graph) const;
    
    // Sampling����getter
    boost::optional<std::size_t> const& sampling_size() const;

private:
    sampler const loader_;
    std::string const file_;
};

basic_info_criteria::basic_info_criteria(std::string const& file)
    : loader_(), file_(file)
{
}

double basic_info_criteria::calc_likelihood(graph_t const& graph) const
{
    double likelihood = 0.0;

    // �T���v����1�s���ǂݍ���
    loader_.load_sample(
        graph, file_,
        [&likelihood, &graph](condition_t const& sample)
        {
            condition_t cond = sample;

            // �e�m�[�h�ɑ΂�
            for(auto const& node : graph.vertex_list())
            {
				for (auto it = cond.begin(); it != cond.end();)
				{
					auto const& parent = graph.in_vertexes(node);
					if (std::find(parent.begin(), parent.end(), it->first) == parent.end())
					{
						it = cond.erase(it);
					}
					else
					{
						++it;
					}
				}
				for (int i = 0; i < node->selectable_num; i++)
                {
					if (node->cpt[cond].second[i] != 0)
                    {
						likelihood -= std::log2(node->cpt[cond].second[i]);
					}
				}
            }
        });

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
                return init * (parent_node->selectable_num - 1);
            });
    }

    return static_cast<double>(parameters);
}

boost::optional<std::size_t> const& basic_info_criteria::sampling_size() const
{
    return loader_.sampling_size();
}

} // namespace evaluation
} // namespace bn

#endif // #ifndef BNI_EVALUATION_BASIC_INFO_CRITERIA_HPP
