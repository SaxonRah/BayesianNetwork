#ifndef BNI_EVALUATION_TRANSINFORMATION_HPP
#define BNI_EVALUATION_TRANSINFORMATION_HPP

#include <bayesian/graph.hpp>
#include <bayesian/sampler.hpp>

namespace bn {
namespace evaluation {

struct entropy {
    // sampling���瓾����e�[�u����p���āCvariables�̓���entropy���v�Z����
    // sampling: load_sample�ς�sampler
    // variables: ���ׂ�m�[�h�̃��X�g
    double operator() (sampler const& sampling, std::vector<vertex_type> const& variables) const
    {
        // ���f�[�^���猵���Ȋm�����o��(�����t���m���ł͂Ȃ�)
        // CPT��e�m�[�h���s���Z���ĎZ�o���邱�Ƃ��\�����C�ǂ����낤��
        std::unordered_map<condition_t, std::size_t> table;
        for(auto const& sample : sampling.table())
        {
            condition_t cond;
            for(auto const& variable : variables)
                cond[variable] = sample.first.at(variable);

            // �L��Ή��Z�C�Ȃ���΍쐬
            auto it = table.find(cond);
            if(it == table.end()) table.emplace(cond, sample.second);
            else                  it->second += sample.second;
        }

        // ���ׂĂ̊m���ɒ�2�̃G���g���s�[�v�Z���s��
        double entropy = 0.0;
        for(auto const& pattern : table)
        {
            auto const probability = static_cast<double>(pattern.second) / sampling.sampling_size();
            entropy -= probability * std::log2(probability);
        }
        return entropy;
    }

    // sampling���瓾����e�[�u����p���āCvariables�̓���entropy���v�Z����
    // sampling: load_sample�ς�sampler
    // variable: ���ׂ�m�[�h
    double operator() (sampler const& sampling, vertex_type const& variable) const
    {
        std::vector<vertex_type> variables = {variable};
        return (*this)(sampling, variables);
    }
};

struct mutual_information {
    // sampling���瓾����e�[�u����p���āCx��y�̊Ԃ̑��ݏ��ʂ��v�Z����
    // sampling: load_sample�ς�sampler
    // x, y: ���ׂ�m�[�h
    double operator() (sampler const& sampling, vertex_type const& x, vertex_type const& y) const
    {
        entropy ent;
        return ent(sampling, x) + ent(sampling, y) - ent(sampling, {x, y});
    }

    // sampling���瓾����e�[�u����p���āCx��y�̊Ԃ̑��ݏ��ʂ��v�Z����(x��y�P�̂�entropy���v�Z�ς̏ꍇ)
    // sampling: load_sample�ς�sampler
    // x, y: ���ׂ�m�[�h
    // x_ent, y_ent: ���ׂ�m�[�h�̃G���g���s�[
    template<class T>
    double operator() (sampler const& sampling, vertex_type const& x, T const x_ent, vertex_type const& y, T const y_ent) const
    {
        entropy ent;
        return x_ent + y_ent - ent(sampling, {x, y});
    }

    // sampling���瓾����e�[�u����p���āCx��y�̊Ԃ̑��ݏ��ʂ��v�Z����(x��y��xy��entropy���v�Z�ς̏ꍇ)
    // x_ent, y_ent: ���ׂ�m�[�h�̃G���g���s�[
    // xy_ent: ���ׂ�m�[�h2�̓����G���g���s�[
    template<class T>
    double operator() (T const x_ent, T const y_ent, T const xy_ent) const
    {
        return x_ent + y_ent - xy_ent;
    }
};

} // namespace evaluation
} // namespace bn

#endif // #ifndef BNI_EVALUATION_TRANSINFORMATION_HPP
