<template>
  <ul>
    <component v-for="e in elements" v-bind:key="e" v-bind:is="e.type" v-bind="e.props"/>
  </ul>
</template>

<script>
export default {
  name: 'YattsAccordion',
  props: ['elements']
}
</script>
