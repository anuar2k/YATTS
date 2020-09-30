<template>
  <i class="fas fa-caret-down rotate-icon" v-bind:class="{ rotated: rotated }"></i>
</template>

<script>
export default {
  name: 'RotatingArrow',
  props: ['rotated']
}
</script>

<style lang="scss" scoped>
  .rotate-icon {
    transform: rotate(-90deg);
  }

  .rotate-icon.rotated {
    transform: rotate(0deg);
  }
</style>
