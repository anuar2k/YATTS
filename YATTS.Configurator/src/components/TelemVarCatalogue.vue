<template>
  <div class="row disable-selection card bg-3 mb-2" v-for="category in state" v-bind:key="category.def.id">
    <div class="col">
      <div class="row" v-on:click="$emit('toggle-show', category)">
        <div class="col">
          <span class="category-arrow"><RotatingArrow v-bind:rotated="category.show" /></span>
          <span class="category-name">{{ category.def.name }}</span>
          <span class="category-desc">{{ category.def.description }}</span>
        </div>
      </div>
      <div class="row indent-groups" v-show="category.show">
        <div class="col">
          <div class="row" v-for="group in category.groups" v-bind:key="group.def.id">
            <div class="col">
              <div class="row" v-on:click="$emit('toggle-show', group)">
                <div class="col">
                  <RotatingArrow v-bind:rotated="group.show" />&nbsp;{{ group.def.name }}
                </div>
              </div>
              <div class="row indent-variables" v-show="group.show">
                <div class="col">
                  <div class="row" v-for="variable in group.variables" v-bind:key="variable.def.id">
                    <div class="col" v-on:click="$emit('toggle-selected', variable)">
                      {{ variable.def.id }}, selected: {{ variable.selected }}
                    </div>
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import RotatingArrow from './RotatingArrow';

export default {
  name: 'TelemVarCatalogue',
  props: ['state'],
  emits: ['toggle-show', 'toggle-selected'],
  mounted() {
    console.log('siemka');
  },
  components: {
    RotatingArrow
  }
}
</script>

<style lang="scss" scoped>

  .indent-groups {
    padding-left: 1.2rem;
  }

  .indent-variables {
    padding-left: 2rem;
  }

  .disable-selection {
    user-select: none;
  }

  .category-arrow {
    font-size: medium;
    margin-right: .5rem;
    vertical-align: 12%;
  }

  .category-name {
    font-size: x-large;
    margin-right: .5rem;
  }

  .category-desc {
    font-size: medium;
  }
</style>
